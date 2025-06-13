#ifndef STRING_H
#define STRING_H
#include <iostream>
#include <string>
#include "MyVector.h"
using namespace std;

int mystrlen(const char* str)
{
    int length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

char* mystrstr(const char* haystack, const char* needle)
{
	if (*needle == '\0')
	{
		return (char*)haystack;
	}

	int haystackLen = mystrlen(haystack);
	int needleLen = mystrlen(needle);

	for (int i = 0; i <= haystackLen - needleLen; i++)
	{
		int j = 0;
		while (j < needleLen && haystack[i + j] == needle[j])
		{
			j++;
		}

		if (j == needleLen)
		{
			return (char*)(haystack + i);
		}
	}

	return nullptr;
}


char* mystrcpy(char* dest, const char* src)
{
    int i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

char* mystrncpy(char* dest, const char* src, int n)
{
    int i = 0;
    while (i < n && src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    while (i < n)
    {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

int mystrcmp(const char* str1, const char* str2)
{
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0')
    {
        if (str1[i] != str2[i])
        {
            return str1[i] - str2[i];
        }
        i++;
    }
    return str1[i] - str2[i];
}

char* mystrcat(char* dest, const char* src)
{
    int i = 0;
    while (dest[i] != '\0')
    {
        i++;
    }
    int j = 0;
    while (src[j] != '\0')
    {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';
    return dest;
}

class MyString
{
private:
	char* data;
	int size;

public:
	MyString()
	{
		size = 0;
		data = new char[1];
		data[0] = '\0';
	}

	MyString(const char* str)
	{
		size = mystrlen(str);
		data = new char[size + 1];
		mystrcpy(data, str);
	}

	MyString(const string& str)
	{
		size = str.length();
		data = new char[size + 1];
		mystrcpy(data, str.c_str());
	}

	MyString(const MyString& other)
	{
		size = other.size;
		data = new char[size + 1];
		mystrcpy(data, other.data);
	}

	~MyString()
	{
		delete[] data;
	}

	MyString& operator=(const MyString& other)
	{
		if (this != &other)
		{
			delete[] data;
			size = other.size;
			data = new char[size + 1];
			mystrcpy(data, other.data);
		}
		return *this;
	}

	MyString operator+(const MyString& other) const
	{
		MyString result;
		result.size = size + other.size;
		delete[] result.data;
		result.data = new char[result.size + 1];
		mystrcpy(result.data, data);
		mystrcat(result.data, other.data);
		return result;
	}

	bool operator==(const MyString& other) const
	{
		return mystrcmp(data, other.data) == 0;
	}

	bool operator!=(const MyString& other) const
	{
		return !(*this == other);
	}

	int length() const
	{
		return size;
	}

	bool empty() const
	{
		return size == 0;
	}

	MyString substr(int pos, int len) const
	{
		if (pos < 0 || pos >= size)
		{
			return MyString("");
		}

		if (pos + len > size)
		{
			len = size - pos;
		}

		char* temp = new char[len + 1];
		mystrncpy(temp, data + pos, len);
		temp[len] = '\0';

		MyString result(temp);
		delete[] temp;
		return result;
	}

	int find(const MyString& str) const
	{
		char* pos = mystrstr(data, str.data);
		if (pos)
			return pos - data;
		return -1;
	}

	int lastIndexOf(char ch) const
	{
		for (int i = size - 1; i >= 0; i--)
		{
			if (data[i] == ch)
				return i;
		}
		return -1;
	}

	void insert(int pos, const MyString& str)
	{
		if (pos > size) pos = size;

		char* newData = new char[size + str.size + 1];
		mystrncpy(newData, data, pos);
		mystrcpy(newData + pos, str.data);
		mystrcpy(newData + pos + str.size, data + pos);

		delete[] data;
		data = newData;
		size += str.size;
	}

	int compare(const MyString& other) const
	{
		return mystrcmp(data, other.data);
	}

	void replace(int pos, int len, const MyString& str)
	{
		if (pos > size) return;
		if (pos + len > size) len = size - pos;

		char* newData = new char[size - len + str.size + 1];
		mystrncpy(newData, data, pos);
		mystrcpy(newData + pos, str.data);
		mystrcpy(newData + pos + str.size, data + pos + len);
		delete[] data;
		data = newData;
		size = size - len + str.size;
	}

	const char* c_str() const
	{
		return data;
	}

	char operator[](int index) const
	{
		if (index >= 0 && index < size)
			return data[index];
		return '\0';
	}

	friend ostream& operator<<(ostream& os, const MyString& s)
	{
		os << s.data;
		return os;
	}
};

string trimString(const string& str)
{
	size_t first = 0;
	while (first < str.length() && (str[first] == ' ' || str[first] == '\t' || str[first] == '\n' || str[first] == '\r'))
	{
		++first;
	}
	if (first == str.length())
	{
		return "";
	}
	size_t last = str.length() - 1;
	while (last > first && (str[last] == ' ' || str[last] == '\t' || str[last] == '\n' || str[last] == '\r'))
	{
		--last;
	}
	return str.substr(first, (last - first + 1));
}

vector<string> splitString(const string& str, char delimiter)
{
	vector<string> tokens;
	string token;

	for (size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] == delimiter)
		{
			tokens.push_back(trimString(token));
			token.clear();
		}
		else
		{
			token += str[i];
		}
	}

	if (!token.empty() || !str.empty())
	{
		tokens.push_back(trimString(token));
	}

	return tokens;
}

#endif
