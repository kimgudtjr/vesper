/**----------------------------------------------------------------------------
 * test_analyzer.cpp
 *-----------------------------------------------------------------------------
 * test for analyzer class 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:3:21 14:32 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "gtest\gtest.h"
#include "..\analyzer.h"

/**
 * @brief	fixture / teardown 
**/
class TestAnalyzer: public testing::Test 
{
protected: 
	virtual void SetUp() 
	{		
		ASSERT_TRUE(_analyzer.initialize(L"d:\\work.git\\vesper\\build\\x64_debug\\20140221_165811.db"));
	}

	virtual void TearDown() 
	{
		_analyzer.finalize();
	}

	// Declares the variables your tests want to use.
	analyzer	_analyzer;
	
};

/**
 * @brief	
**/
TEST_F(TestAnalyzer, load_module_symbols)
{
	ASSERT_EQ(true, _analyzer.load_module_symbols());
}

