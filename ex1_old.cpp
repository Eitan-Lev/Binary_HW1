#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>
#include <string>

using namespace::std;

/* ===================================================================== */
/* Global Variables                                                      */
/* ===================================================================== */

//Using 2 map objects, for name and counts
map <UINT32, UINT32> RTNTABLE;
map <UINT32, string> RTNNAMES;

class RoutineData {
public:
  RTN Routine;
  IMG Img;
  UINT32 Count;
  RoutineData(RTN rtn, IMG img)
    : Routine(rtn), Img(img), Count(0) {}
}

//RoutineData rtnData;

map <UINT32, RoutineData> RoutineMap;



struct RoutineData2 {
  RTN Routine;
  IMG Img;
  UINT32 Count;
  //map <UINT32, RTN> _RTN_MAP;
}


/* ===================================================================== */
/* Help Message                                                          */
/* ===================================================================== */
INT32 Usage()
{
    cerr <<
        "This tool prints out the number of dynamic instructions executed to stderr"
		<< endl << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}

/* ===================================================================== */

VOID PIN_FAST_ANALYSIS_CALL docount(UINT32 count, UINT32 RoutineID) { RTNTABLE[RoutineID] += count; }

/* ===================================================================== */

VOID BasicBlock(TRACE trc , VOID *v)
{
	//BBL head
	for (BBL bbl =TRACE_BblHead(trc); BBL_Valid(bbl); bbl = BBL_Next(bbl))
	{
		RTN CurrentRTN = RTN_FindByAddress(BBL_Address(bbl));
		UINT32 CurrentId = RTN_Id(CurrentRTN);

    ADDRINT RoutinAddress = RTN_Address(CurrentRTN);

    IMG CurrentImg = IMG_FindByAddress(BBL_Address(bbl));
    string ImgName = IMG_Name(CurrentImg);
    ADDRINT ImgAddress = IMG_Entry(CurrentImg);//Check if right function
    if (RoutineMap.find(CurrentId) == RoutineMap.end()) {
        RoutineData rtnData = new RoutineData(CurrentRTN, CurrentImg);
        RoutineMap.insert(pair<UINT32,RoutineData>(CurrentID,rtnData));
        RTNTABLE.insert(pair<UINT32,UINT32>(CurrentId,0));  //create new one
    }
		//if(RTNTABLE.find(CurrentId) == RTNTABLE.end()) //not exist
			//RTNTABLE.insert(pair<UINT32,UINT32>(CurrentId,0));  //create new one

		// check is the element already exist in RTNNAMES map
		if(RTNNAMES.find(CurrentId) == RTNNAMES.end()) //not exist
			RTNNAMES.insert(pair<UINT32,string>(CurrentId,RTN_FindNameByAddress(BBL_Address(bbl))));  	//create new one

		BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, IARG_FAST_ANALYSIS_CALL, IARG_UINT32, BBL_NumIns(bbl), IARG_UINT32, CurrentRTN, IARG_END);
	}
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
	//open file
	ofstream oFile;
	oFile.open("rtn-output.txt");
	unsigned int size_l = RTNTABLE.size();

 cout << RTNTABLE.size() << endl << RTNNAMES.size() << endl;

 //iterators for map
 map<UINT32,string>::iterator it_CurrentName;
 map<UINT32,UINT32>::iterator it_CurrentNumber;

	unsigned int max = 0;
	for (unsigned int i=0;i<size_l;i++) //check all elements
	{
		for (map<UINT32,UINT32>::iterator iter = RTNTABLE.begin(); iter != RTNTABLE.end(); ++iter)
		{
			if((unsigned int)iter->second > max) //find biggest element
			{
				max = (unsigned int)iter->second;
				it_CurrentNumber = iter;
			}
		}
    it_CurrentName = RTNNAMES.find(it_CurrentNumber->first);
	oFile << '"' << it_CurrentName->second << '"' << " icount " << it_CurrentNumber->second << endl;
	// remove current element. the next one will be smaller...
	RTNTABLE.erase(it_CurrentNumber);
    RTNNAMES.erase(it_CurrentName);
    max = 0; //to compare next time
	}
	oFile.close(); //close file
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
	// Initialize pin and pin symbol table code
	PIN_InitSymbols();
	// Initialize pin
	if( PIN_Init(argc,argv) )
		return Usage();

	TRACE_AddInstrumentFunction(BasicBlock, 0);
	// Register Fini to be called when the application exits
	PIN_AddFiniFunction(Fini, 0);
	// Start the program, never returns
	PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
