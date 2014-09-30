/*
 * CSVReader.cpp
 *
 *  Created on: Feb 12, 2014
 *      Author: qzhao2
 */

#include "csv.h"

istream& operator>>(istream& str, CSVRow& data) {
	data.readNextRow(str);
	return str;
}
