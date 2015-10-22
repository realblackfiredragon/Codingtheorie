#include <set>
#include <map>
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>
#include <assert.h>
#include <queue>
#include <fstream>
#include <sstream>

template <class T>
class InformationSource
{
public:
	InformationSource(const std::vector<T>& input)
	{
		const int n = input.size();
		std::unordered_map<T, int> appearances;

		for (const auto& e : input)
		{
			if (appearances.count(e) == 0)
			{
				appearances[e] = 1;
			}
			else
			{
				appearances[e]++;
			}
		}

		for (auto e : appearances)
		{
			mProbabilities[e.first] = float(e.second) / float(n);
		}
	}

	const std::unordered_map<T, float>& getPropabilities() const
	{
		return mProbabilities;
	}
private:
	std::unordered_map<T, float> mProbabilities;
};

template <class T>
class HuffmanCode
{

public:
	//
	//	Helper Classes
	//
	struct HuffmanCodeNode
	{
		HuffmanCodeNode(HuffmanCodeNode* left, HuffmanCodeNode* right, float probability)
			:mLeft(left)
			, mRight(right)
			, mProbability(probability)
		{}
		
		~HuffmanCodeNode()
		{
			delete mLeft;
			delete mRight;
		}

		HuffmanCodeNode* mLeft;
		HuffmanCodeNode* mRight;
		float mProbability;
	};

	struct HuffmanCodeLeaf : HuffmanCodeNode
	{
		HuffmanCodeLeaf(T data, float probability)
			: mData(data)
			, HuffmanCodeNode(nullptr, nullptr, probability)
		{}
		T mData;
	};


public:
	HuffmanCode(InformationSource<T> is)
	{
		std::multimap<float, HuffmanCodeNode*> nodes;

		for (std::pair<T, float> e : is.getPropabilities())
		{
			HuffmanCodeNode* node = new HuffmanCodeLeaf(e.first, e.second);
			nodes.emplace(e.second, node);
		}

		while (nodes.size() > 1)
		{
			auto n1 = *nodes.begin();
			nodes.erase(nodes.begin());
			auto n2 = *nodes.begin();
			nodes.erase(nodes.begin());
			
			auto node = new HuffmanCodeNode(n1.second, n2.second, n1.first + n2.first);
			nodes.emplace(node->mProbability, node);
		}

		mRoot = (*nodes.begin()).second;
	}

	HuffmanCodeNode* getRoot()
	{
		return mRoot;
	}

private:
	HuffmanCodeNode* mRoot;
};

template <class T>
class HuffmanEncoder
{
public:
	HuffmanEncoder(const InformationSource<T>& in)
	{
		HuffmanCode<T> code(in);

		buildMapping(code.getRoot());
	}

private:

	void buildMapping(typename const HuffmanCode<T>::HuffmanCodeNode* root)
	{
		std::queue<const HuffmanCode<T>::HuffmanCodeNode*> nodesToTraverse;
		std::queue < std::vector<bool> > runningCodes;
		nodesToTraverse.push(root);
		runningCodes.push(std::vector<bool>());
		while (nodesToTraverse.size())
		{
			auto n = nodesToTraverse.front();
			nodesToTraverse.pop();
			auto code = runningCodes.front();
			runningCodes.pop();
			if (n->mLeft != nullptr)
			{
				std::vector<bool> c1(code.begin(), code.end());
				std::vector<bool> c2(code.begin(), code.end());
				c1.push_back(false);
				c2.push_back(true);
				runningCodes.push(std::move(c1));
				runningCodes.push(std::move(c2));
				nodesToTraverse.push(n->mLeft);
				nodesToTraverse.push(n->mRight);
			}
			else
			{
				mMapping.emplace(static_cast<const HuffmanCode<T>::HuffmanCodeLeaf*>(n)->mData, code);
			}

		}

	}

public:
	void operator()(std::ostream& out, std::vector<T> message)
	{
		unsigned char activeByte = 0;
		short filledBits = 0;


		for (auto symbol : message)
		{
			auto code = mMapping[symbol];
			for (unsigned short i = 0; i < code.size(); i++)
			{
				if (code[i])
				{
					activeByte |= 1 << (7 - filledBits);
				}
				filledBits++;

				if (filledBits == 8)
				{
					out << activeByte;
					activeByte = 0;
					filledBits = 0;
				}
			}
		}

		if (filledBits != 0)
		{
			out << activeByte;
		}
	}

	std::map<T, std::vector<bool>> mMapping;
};

template <class T>
class HuffmanDecoder
{
public:
	HuffmanDecoder(const InformationSource<char> is)
		:mCode(is)
	{
		mLastNodeTraversed = mCode.getRoot();
	}

	void operator()(std::ostream& out, std::istream& in)
	{
		unsigned char byteRead;
		while (in.get(byteRead))
		{
			traverseTree(byteRead, out);
		}
	}

	bool traverseTree(const unsigned char byteRead, std::ostream& out) const
	{
		std::vector<bool> bitset(byteRead);

		for (auto bit : bitset)
		{
			mLastNodeTraversed = bit ? mLastNodeTraversed->mRight : mLastNodeTraversed->mLeft;

			if (mLastNodeTraversed->mLeft == nullptr) //We hit a leaf, so print it.
			{
				out << static_cast<HuffmanCode<T>::HuffmanCodeLeaf*>(mLastNodeTraversed)->mData;
				mLastNodeTraversed = mCode.getRoot();
			}
		}
	}


	typename HuffmanCode<T>::HuffmanCodeNode* mLastNodeTraversed;
	HuffmanCode<T> mCode;
};

void main(int argc, char** argv)
{
	const std::string str = "Blubb";
	std::stringstream ssClear;
	ssClear.str(str);

	std::stringstream ssEncoded;
	std::stringstream ssDecoded;

	std::vector<char> input(str.begin(), str.end());
	input.push_back('\0');
	InformationSource<char> blubb(input);

	HuffmanEncoder<char> encoder(blubb);
	std::ofstream test("test.txt", std::ofstream::out);

	encoder(test, input);
	
}