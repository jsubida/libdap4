// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2014 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#include "config.h"

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include <sstream>

#include "Byte.h"
#include "Int16.h"
#include "UInt16.h"
#include "Int32.h"
#include "UInt32.h"
#include "Float32.h"
#include "Float64.h"
#include "Str.h"
#include "Url.h"
#include "Array.h"
#include "Structure.h"
#include "Sequence.h"
#include "Grid.h"

#include "DDS.h"
#include "DMR.h"
#include "XMLWriter.h"
#include "D4BaseTypeFactory.h"
#include "D4ParserSax2.h"

#include "GNURegex.h"
#include "GetOpt.h"
#include "util.h"
#include "mime_util.h"
#include "debug.h"
#include "GetOpt.h"

#include "testFile.h"
#include "test_config.h"

static bool debug = false;
static bool mo_debug = false;

#undef DBG
#define DBG(x) do { if (debug) {x;} } while(false)

using namespace CppUnit;
using namespace std;

static string THE_TESTS_DIR("/dmr-to-dap2-testsuite/");

namespace libdap {

class DmrToDap2Test: public TestFixture {
private:

public:
    DmrToDap2Test() {
    }
    ~DmrToDap2Test() {
    }

    void setUp() {
    }

    void tearDown() {
    }

    bool re_match(Regex &r, const string &s) {
        int match = r.match(s.c_str(), s.length());
        DBG(cerr << "Match: " << match << " should be: " << s.length() << endl);
        return match == static_cast<int> (s.length());
    }

    /**
     * Given the name of a DDS and optional DAS file, build a DMR using the
     * hackery known as transform_to_dap4 and the new DMR ctor.
     *
     * @param dds_file
     * @param attr
     * @return A pointer to the new DMR; caller must delete
     */
    DMR *build_dmr(const string &dmr_file) {
        DBG(cerr << __func__ << "() - BEGIN" << endl);
        DBG(cerr << __func__ << "() - dmr_file: " << dmr_file << endl);

		try {
			DMR *dmr =  new DMR();
		    dmr->set_filename(dmr_file);
		    dmr->set_name(name_path(dmr_file));
		    D4BaseTypeFactory BaseFactory;   // Use the factory for this handler's types
		    dmr->set_factory(&BaseFactory);
		    D4ParserSax2 parser;
		    ifstream in(dmr_file.c_str(), ios::in);
		    parser.intern(in, dmr, mo_debug);
		    dmr->set_factory(0);
	        DBG(cerr << __func__ << "() - END" << endl);
			return dmr;
		}
    	catch (Error &e) {
    		CPPUNIT_FAIL(string("Caught Error: ") + e.get_error_message());
    	}

    	return 0;
    }

    void test_template(const string &test_base_name) {

        string prefix = string(TEST_SRC_DIR) + THE_TESTS_DIR;

        string dmr_file = prefix + test_base_name + ".dmr";
        string dds_file = prefix + test_base_name + ".dds";
        string das_file = prefix + test_base_name + ".das";

        DBG(cerr << __func__ << "() - BEGIN (test_base: "<< test_base_name << ")" << endl);
        DMR *dmr = 0;
        try {

            dmr = build_dmr(dmr_file);
            CPPUNIT_ASSERT(dmr!=0);
            XMLWriter xml;
            dmr->print_dap4(xml);
            string result_dmr(xml.get_doc());
            string baseline_dmr = readTestBaseline(dmr_file);

            DBG(cerr << "BASELINE DMR("<< baseline_dmr.size() << " chars): " << dmr_file << endl << baseline_dmr << endl);
            DBG(cerr << "RESULT DMR("<< result_dmr.size() << " chars): " << endl << result_dmr << endl);
            CPPUNIT_ASSERT(result_dmr == baseline_dmr);

            DDS *dds =  dmr->getDDS();
            std::ostringstream result_dds;
            dds->print(result_dds);
            string baseline_dds = readTestBaseline(dds_file);
            DBG(cerr << "BASELINE DDS("<< baseline_dds.size() << " chars): " << dds_file << endl << baseline_dds << endl);
            DBG(cerr << "RESULT DDS("<< result_dds.str().size() << " chars): " << endl << result_dds.str() << endl);
            CPPUNIT_ASSERT(result_dds.str() == baseline_dds);

            std::ostringstream result_das;
            dds->print_das(result_das);
            string source_das = readTestBaseline(das_file);
            DBG(cerr << "BASELINE DAS("<< source_das.size() << " chars): " << das_file << endl << source_das << endl);
            DBG(cerr << "RESULT DAS("<< result_das.str().size() << " chars): " << endl << result_das.str() << endl);
            CPPUNIT_ASSERT(result_das.str() == source_das);

            delete dmr;
            delete dds;
        }
        catch (Error &e) {
            delete dmr;
            CPPUNIT_FAIL(string("Caught Error: ") + e.get_error_message());
        }
        DBG(cerr << __func__ << "() - END" << endl);
    }


    // Test a DDS with simple scalar types and no attributes
    void dmr_to_dap2_01() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("test.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    // Test a DDS with simple scalar types and no attributes
    void dmr_to_grid_01() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("coads_climatology.nc");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void basic_dmr_to_dap2_0_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_0.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void basic_dmr_to_dap2_0_1() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_0.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void basic_dmr_to_dap2_1_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_1.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }


    void basic_dmr_to_dap2_2_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_2.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void basic_dmr_to_dap2_2_1() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_2.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void basic_dmr_to_dap2_2_2() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_2.2");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void basic_dmr_to_dap2_2_3() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_2.3");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void basic_dmr_to_dap2_2_4() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_2.4");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void basic_dmr_to_dap2_2_5() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_2.5");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void basic_dmr_to_dap2_3_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("DMR_3.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }


    void enum_dmr_to_dap2_0_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_0.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void enum_dmr_to_dap2_0_1() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_0.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void enum_dmr_to_dap2_1_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_1.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void enum_dmr_to_dap2_1_1() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_1.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void enum_dmr_to_dap2_1_2() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_1.2");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void enum_dmr_to_dap2_1_3() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_1.3");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void enum_dmr_to_dap2_1_4() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_1.4");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void enum_dmr_to_dap2_1_5() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_1.5");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void enum_dmr_to_dap2_2_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_2.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void enum_dmr_to_dap2_2_1() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_2.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void enum_dmr_to_dap2_3_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_3.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void enum_dmr_to_dap2_3_1() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_3.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void enum_dmr_to_dap2_4_0() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_4.0");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    void enum_dmr_to_dap2_4_1() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_4.1");
        DBG(cerr << __func__ << "() - END" << endl);
    }
    void enum_dmr_to_dap2_4_2() {
        DBG(cerr << endl << __func__ << "() - BEGIN" << endl);
        test_template("EnumDMR_4.2");
        DBG(cerr << __func__ << "() - END" << endl);
    }

    CPPUNIT_TEST_SUITE( DmrToDap2Test );


#if 1 // good
    CPPUNIT_TEST(dmr_to_dap2_01);
    CPPUNIT_TEST(basic_dmr_to_dap2_0_0);
    CPPUNIT_TEST(basic_dmr_to_dap2_0_1);
    CPPUNIT_TEST(basic_dmr_to_dap2_2_0);
    CPPUNIT_TEST(basic_dmr_to_dap2_2_1);
    CPPUNIT_TEST(basic_dmr_to_dap2_2_2);
    CPPUNIT_TEST(basic_dmr_to_dap2_2_4);
    CPPUNIT_TEST(basic_dmr_to_dap2_2_3);
    CPPUNIT_TEST(basic_dmr_to_dap2_2_5); // Drops Variables

    CPPUNIT_TEST(enum_dmr_to_dap2_0_0);
    CPPUNIT_TEST(enum_dmr_to_dap2_0_1);
    CPPUNIT_TEST(enum_dmr_to_dap2_1_0);
    CPPUNIT_TEST(enum_dmr_to_dap2_1_1);
    CPPUNIT_TEST(enum_dmr_to_dap2_1_2);
    CPPUNIT_TEST(enum_dmr_to_dap2_1_3);
    CPPUNIT_TEST(enum_dmr_to_dap2_1_4);
    CPPUNIT_TEST(enum_dmr_to_dap2_2_0); // Drops Variables
    CPPUNIT_TEST(enum_dmr_to_dap2_2_1); // Drops Variables
    CPPUNIT_TEST(enum_dmr_to_dap2_3_0); // Drops Variables
    CPPUNIT_TEST(enum_dmr_to_dap2_3_1); // Drops Variables

    CPPUNIT_TEST(enum_dmr_to_dap2_4_0); // Drops Groups
    CPPUNIT_TEST(enum_dmr_to_dap2_4_1); // Drops Groups
    CPPUNIT_TEST(enum_dmr_to_dap2_4_1);
    CPPUNIT_TEST(enum_dmr_to_dap2_4_2);

#endif


//    CPPUNIT_TEST(dmr_to_grid_01);




#if 0 // bad
    CPPUNIT_TEST(enum_dmr_to_dap2_1_5); // Parser issue with look-ahead




#endif


    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(DmrToDap2Test);


} // namepsace libdap

int main(int argc, char*argv[]) {
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    GetOpt getopt(argc, argv, "d");
    int option_char;

    while ((option_char = getopt()) != -1)
        switch (option_char) {
        case 'd':
            debug = 1;  // debug is a static global
            break;
        case 'D':
            mo_debug = 1;  // debug is a static global
            break;
        default:
            break;
        }

    bool wasSuccessful = true;
    string test = "";
    int i = getopt.optind;
    if (i == argc) {
        // run them all
        wasSuccessful = runner.run("");
    }
    else {
        while (i < argc) {
            test = string("libdap::DMRTest::") + argv[i++];
            DBG(cerr << "test: " << test << endl);
            wasSuccessful = wasSuccessful && runner.run(test);
        }
    }

    xmlMemoryDump();

    return wasSuccessful ? 0 : 1;
}

