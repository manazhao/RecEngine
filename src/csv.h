/*
 * CSVReader.h
 *
 *  Created on: Feb 12, 2014
 *      Author: qzhao2
 */

#ifndef CSVREADER_H_
#define CSVREADER_H_

#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

class CSVRow {
public:
	CSVRow(char delimiter = ',') :
			m_delimiter(delimiter) {
	}
	string const& operator[](size_t index) const {
		return m_data[index];
	}
	size_t size() const {
		return m_data.size();
	}
	void readNextRow(istream& str) {
		string line;
		getline(str, line);
		stringstream lineStream(line);
		string cell;
		m_data.clear();
		while (getline(lineStream, cell, m_delimiter)) {
			m_data.push_back(cell);
		}
	}
private:
	vector<string> m_data;
	char m_delimiter;
};

istream& operator>>(istream& str, CSVRow& data);

class CSVIterator {
public:
	typedef input_iterator_tag iterator_category;
	typedef CSVRow value_type;
	typedef size_t difference_type;
	typedef CSVRow* pointer;
	typedef CSVRow& reference;

	CSVIterator(istream& str, char delimiter = ',') :
			m_strPtr(str.good() ? &str : NULL), m_row(delimiter) {
		++(*this);
	}
	CSVIterator() :
			m_strPtr(NULL) {
	}
	/// pre Increment
	CSVIterator& operator++() {
		if (m_strPtr) {
			(*m_strPtr) >> m_row;
			m_strPtr = (*m_strPtr).good() ? m_strPtr : NULL;
		}
		return *this;
	}
	CSVIterator operator++(int) {
		CSVIterator tmp(*this);
		++(*this);
		return tmp;
	}

	/// row data is not changable
	CSVRow const& operator*() const {
		return m_row;
	}

	CSVRow const* operator->() const {
		return &m_row;
	}
	bool operator==(CSVIterator const& rhs) {
		return ((this == &rhs)
				|| (this->m_strPtr == NULL && rhs.m_strPtr == NULL));
	}
	bool operator!=(CSVIterator const& rhs) {
		return !((*this) == rhs);
	}
private:
	istream *  m_strPtr;
	CSVRow m_row;
};

#endif /* CSVREADER_H_ */
