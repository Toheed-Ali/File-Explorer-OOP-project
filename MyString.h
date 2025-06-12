#ifndef STRING_H
#define STRING_H
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

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
		size = strlen(str);
		data = new char[size + 1];
		strcpy(data, str);
	}

	MyString(const string& str)
	{
		size = str.length();
		data = new char[size + 1];
		strcpy(data, str.c_str());
	}

	MyString(const MyString& other)
	{
		size = other.size;
		data = new char[size + 1];
		strcpy(data, other.data);
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
			strcpy(data, other.data);
		}
		return *this;
	}

	MyString operator+(const MyString& other) const
	{
		MyString result;
		result.size = size + other.size;
		delete[] result.data;
		result.data = new char[result.size + 1];
		strcpy(result.data, data);
		strcat(result.data, other.data);
		return result;
	}

	bool operator==(const MyString& other) const
	{
		return strcmp(data, other.data) == 0;
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
		strncpy(temp, data + pos, len);
		temp[len] = '\0';

		MyString result(temp);
		delete[] temp;
		return result;
	}

	MyString substring(int pos, int len) const
	{
		return substr(pos, len);
	}

	int find(const MyString& str) const
	{
		char* pos = strstr(data, str.data);
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
		strncpy(newData, data, pos);
		strcpy(newData + pos, str.data);
		strcpy(newData + pos + str.size, data + pos);

		delete[] data;
		data = newData;
		size += str.size;
	}

	int compare(const MyString& other) const
	{
		return strcmp(data, other.data);
	}

	void replace(int pos, int len, const MyString& str)
	{
		if (pos > size) return;
		if (pos + len > size) len = size - pos;

		char* newData = new char[size - len + str.size + 1];
		strncpy(newData, data, pos);
		strcpy(newData + pos, str.data);
		strcpy(newData + pos + str.size, data + pos + len);
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
