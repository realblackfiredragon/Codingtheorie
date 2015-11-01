#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <memory>

using namespace std;

void ComputeQFromString(const string& rStr, map<char, double>& rQ)
{
	map<char, int> charCount;

	// count chars in string
	int iStrLen = rStr.length();

	for (int iI = 0; iI < iStrLen; iI++)
	{
		pair<map<char, int>::iterator, bool> InsertionResult;

		char cChar = rStr.at(iI);

		InsertionResult = charCount.insert(pair<char, int>(cChar, 0));

		// increment char count
		(InsertionResult.first->second)++;
	}

	// calculate Q
	for (auto It : charCount)
	{
		// calculate p
		double dP = static_cast<double>(It.second) / static_cast<double>(iStrLen);

		rQ.insert(pair<char, double>(It.first, dP));
	}
}

double ComputeEntropyFromQ(map<char, double>& rQ)
{
	double dLog2 = log(2);

	double dE = 0.0;

	for (auto It : rQ)
	{
		double dP = It.second;

		double dI = -(log(dP) / dLog2);

		dE += dP * dI;
	}

	return (dE);
}

struct STreeNode
{
	STreeNode* m_pChildRhs;
	STreeNode* m_pChildLhs;

	double m_dP;
	char m_cChar;

	STreeNode(double dP, char cChar,
		STreeNode* pChildLhs = nullptr,
		STreeNode* pChildRhs = nullptr)
	{
		m_dP = dP;
		m_cChar = cChar;
		m_pChildRhs = pChildLhs;
		m_pChildLhs = pChildRhs;
	}
};

int QSortCmpFunc(const void* pRhs, const void* pLhs)
{
	const STreeNode& rNodeLhs = **((const STreeNode**)(pLhs));
	const STreeNode& rNodeRhs = **((const STreeNode**)(pRhs));

	if (rNodeLhs.m_dP > rNodeRhs.m_dP)		{ return (-1); }
	else if (rNodeLhs.m_dP < rNodeRhs.m_dP) { return ( 1); }
	else									{ return ( 0); }

	return (0);
}

STreeNode* ConstructHTreeFromQ(map<char, double>& rQ)
{
	size_t stNumLeafs = rQ.size();

	if (stNumLeafs == 1)
	{
		map<char, double>::iterator It = rQ.begin();
		return (new STreeNode(It->second, It->first));
	}

	// for each x in rQ, create a tree entry
	vector<STreeNode*> NodeVector(stNumLeafs);

	// fill node vector
	size_t stVecIt = 0;
	for (auto It : rQ)
	{
		NodeVector[stVecIt++] = new STreeNode(It.second, It.first);
	}

	// sort vector
	size_t stNumNodes = NodeVector.size();

	while (stNumNodes > 1)
	{
		STreeNode** ppVectorData = NodeVector.data();
		qsort(ppVectorData, stNumNodes, sizeof(STreeNode*), QSortCmpFunc);

		// connect nodes to trees
		size_t stNumTrees = (stNumNodes / 2);
		size_t stNodeRest = (stNumNodes % 2);
		vector<STreeNode*> TempNodeVector(stNumTrees + stNodeRest);

		for (size_t stIt = 0; stIt < stNumTrees; stIt++)
		{
			STreeNode* pChildLhs = NodeVector[stIt * 2 + 0];
			STreeNode* pChildRhs = NodeVector[stIt * 2 + 1];
			double dPSum = (pChildLhs->m_dP + pChildRhs->m_dP);

			STreeNode* pTreeRoot = new STreeNode(dPSum, '\0');

			// append children to parent node
			pTreeRoot->m_pChildLhs = pChildLhs;
			pTreeRoot->m_pChildRhs = pChildRhs;

			TempNodeVector[stIt] = pTreeRoot;
		}

		if (stNodeRest)
		{
			TempNodeVector[stNumTrees] = NodeVector[stNumTrees * 2 + 0];
		}

		NodeVector = move(TempNodeVector);

		// get new vector size
		stNumNodes = NodeVector.size();
	}

	return (NodeVector[0]);
}

void DestroyHTree(STreeNode* pRootNode)
{
	if (pRootNode->m_pChildLhs != nullptr)
	{
		DestroyHTree(pRootNode->m_pChildLhs);
		pRootNode->m_pChildLhs = nullptr;
	}

	if (pRootNode->m_pChildRhs != nullptr)
	{
		DestroyHTree(pRootNode->m_pChildRhs);
		pRootNode->m_pChildRhs = nullptr;
	}

	delete (pRootNode);
}

void PreOrderTraversal(const STreeNode* pRootNode, string& rCodeWord, map<const char, const string>& rC)
{
	if (pRootNode->m_pChildLhs != nullptr)
	{
		string sCodeWord = rCodeWord;
		sCodeWord += "0"; // FIXME: placed here because vc12 has some issues with the + operation
		PreOrderTraversal(pRootNode->m_pChildLhs, sCodeWord, rC);
	}

	if (pRootNode->m_pChildRhs != nullptr)
	{
		string sCodeWord = rCodeWord;
		sCodeWord += "1"; // FIXME: placed here because vc12 has some issues with the + operation
		PreOrderTraversal(pRootNode->m_pChildRhs, sCodeWord, rC);
	}

	if (pRootNode->m_cChar != '\0')
	{
		rC.insert(pair<const char, const string>(pRootNode->m_cChar, rCodeWord.c_str()));
	}
}

void CreateCodebookFromHTree(const STreeNode* pHTreeRoot, map<const char, const string>& rC)
{
	string sCodeWord;
	PreOrderTraversal(pHTreeRoot, sCodeWord, rC);
}

string DecodeBitStream(const STreeNode* pHTreeRoot, const string& rBitStream)
{
	string sStrStream;
	const char* pCursor = rBitStream.c_str();

	const STreeNode* pCurrentNode = pHTreeRoot;

	char cBit = *pCursor;

	while (cBit != '\0')
	{
		if (pCurrentNode->m_cChar == '\0')
		{
			if ('0' == cBit)
			{
				pCurrentNode = pCurrentNode->m_pChildLhs;
			}
			else
			{
				pCurrentNode = pCurrentNode->m_pChildRhs;
			}

			cBit = *(++pCursor);
		}

		if (pCurrentNode->m_cChar != '\0')
		{
			sStrStream += pCurrentNode->m_cChar;
			pCurrentNode = pHTreeRoot;
		}
	}

	return (move(sStrStream));
}

string EncodeStringStream(const map<const char, const string>& rC, const string& rStrStream)
{
	string sBitStream;

	const char* pCursor = rStrStream.c_str();

	char cChar = *pCursor;

	while (cChar != '\0')
	{
		map<const char, const string>::const_iterator FoundEntryIt = rC.find(cChar);

		sBitStream += FoundEntryIt->second;

		cChar = *(++pCursor);
	}

	return (sBitStream);
}

int main(int iArgc, const char** ppcArgv)
{
	if (iArgc < 2)
	{
		fprintf_s(stderr, "Invalid number of arguments!\n");
		return (1);
	}

	string sStrStream;

	// read file
	ifstream IFStream(ppcArgv[1]);

	for (string sLine; getline(IFStream, sLine);)
	{
		sStrStream += sLine;
	}

	IFStream.close();

	map<char, double> Q;

	// compute Q from string
	ComputeQFromString(sStrStream, Q);

	double dEntropy = ComputeEntropyFromQ(Q);

	fprintf_s(stdout, "Entropy: %f\n", dEntropy);

	// create huffman tree
	STreeNode* pHTreeRoot = ConstructHTreeFromQ(Q);
	
	// create codebook from huffman tree
	map<const char, const string> C;
	CreateCodebookFromHTree(pHTreeRoot, C);

	for (auto It : C)
	{
		fprintf_s(stdout, "%c: %s\n", It.first, It.second.c_str());
	}

	double dPSum = 0;
	for (auto It : Q)
	{
		dPSum += It.second;
		fprintf_s(stdout, "%c: %f\n", It.first, It.second);
	}
	fprintf_s(stdout, "Probability sum: %f\n\n", dPSum);

	string sBitStream;
	string sStrStreamDecoded;

	// encode the string using the codebook
	sBitStream = EncodeStringStream(C, sStrStream);

	// decode the string using the huffman tree
	sStrStreamDecoded = DecodeBitStream(pHTreeRoot, sBitStream);

	// print all data
	fprintf_s(stdout, "Input:\n%s\n\n", sStrStream.c_str());
	fprintf_s(stdout, "Encoded:\n%s\n\n", sBitStream.c_str());
	fprintf_s(stdout, "Decoded:\n%s\n", sStrStreamDecoded.c_str());

	DestroyHTree(pHTreeRoot);

	return (0);
}