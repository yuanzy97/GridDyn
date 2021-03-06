#pragma once
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef _ARRAY_DATA_TRANSLATE_H_
#define _ARRAY_DATA_TRANSLATE_H_

#include "arrayData.h"
#include <array>

/** @brief class implementation translation for another arrayData object
 most function are just simple forwarding to the underlying arrayData object
except the assign and at operator which basically means the arrayData can interact with a small subset of a bigger arrayData object
though rowIndex, colIndex, and val will still return the original values.  The intent of this class is not to replace the interactions with another
it is to act as a filter in cases where elements need to be added but the row needs a translation,  using it outside that purpose could lead to issues
*/
template< int CT, class Y=double>
class arrayDataTranslate : public arrayData<Y>
{
public:
	arrayData<Y> *ad;  //!< the matrix to translate to
private:
	std::array<index_t, CT> Trow;         //!< the vector of translations
public:
	/** @brief constructor
	*/
	arrayDataTranslate() 
	{
		Trow.fill((index_t)(-1));
		//setRowLimit(CT);
	};

	void clear() override
	{
		ad->clear();
	};

	void assign(index_t row, index_t col, Y num) override
	{
		//for this to work the assignment must be from a small number to some other index
		//and we do automatic checking of the translation and if it isn't valid don't do the assignment
		if ((row< CT) && (Trow[row] < arrayData<Y>::rowLim))
		{
			ad->assign(Trow[row], col, num);
		}
	};

	count_t size() const override
	{
		return ad->size();
	};

	count_t capacity() const override
	{
		return ad->capacity();
	};

	index_t rowIndex(index_t N) const override
	{
		return ad->rowIndex(N);
	};

	index_t colIndex(index_t N) const override
	{
		return ad->colIndex(N);
	};

	Y val(index_t N) const override
	{
		return ad->val(N);
	};

	void compact() override
	{
		ad->compact();
	}

	void start() override
	{
		ad->start();
	}

	data_triple<Y> next() override
	{
		return ad->next();
	}

	bool moreData() override
	{
		return ad->moreData();
	}

	Y at(index_t rowN, index_t colN) const override
	{
		if ((rowN < CT) && (Trow[rowN] < arrayData<Y>::rowLim))
		{
			return ad->at(Trow[rowN], colN);
		}
		else
		{
			return 0.0;
		}
	};
	/** set the arrayData object to translate to
	@param[in] newAd  the new arrayData object
	*/
	void setArray(arrayData<Y> *newAd)
	{
		ad = newAd;
		arrayData<Y>::colLim = ad->colLimit();
		arrayData<Y>::rowLim = ad->rowLimit();
	}
	/** set the translation array
	@param[in] input  the input row to translate
	@param[in] output the rowIndex to that input should be translated to
	*/
	void setTranslation(index_t input, index_t output)
	{
		if (input < CT)
		{
			Trow[input] = output;
		}
	}
};

#endif
