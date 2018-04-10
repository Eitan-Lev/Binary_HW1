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
};

//RoutineData rtnData;
map <UINT32, RoutineData> RoutineMap;




struct RoutineData2 {
  RTN Routine;
  IMG Img;
  UINT32 Count;
  //map <UINT32, RTN> _RTN_MAP;
};


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

    //ADDRINT RoutinAddress = RTN_Address(CurrentRTN);

    IMG CurrentImg = IMG_FindByAddress(BBL_Address(bbl));
    //string ImgName = IMG_Name(CurrentImg);
    //ADDRINT ImgAddress = IMG_Entry(CurrentImg);//Check if right function
    if (RoutineMap.find(CurrentId) == RoutineMap.end()) {
        RoutineData rtnData(CurrentRTN, CurrentImg);
        //RoutineData* rtnData = new RoutineData(CurrentRTN, CurrentImg);
        RoutineMap.insert(pair<UINT32,RoutineData>(CurrentId,rtnData));

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
	oFile.open("rtn-output.csv");
  map <UINT32,RoutineData> MaxMap;
  for (map<UINT32,RoutineData>::iterator iter = RoutineMap.begin(); iter != RoutineMap.end(); ++iter) {
    UINT32 Count = (iter->second).Count;
    MaxMap.insert(pair<UINT32,RoutineData>(Count,iter->second));
  }
  for (map<UINT32,RoutineData>::iterator iter = MaxMap.begin(); iter != MaxMap.end(); ++iter) {
    RTN Routine = (iter->second).Routine;
    IMG Img = (iter->second).Img;
    string ImgAddress = "0x" + IMG_Entry(Img);
    string ImgName = IMG_Name(Img);
    string RoutineAddress = "0x" + RTN_Address(Routine);
    string RoutineName = RTN_Name(Routine);
    UINT32 Count = iter->first;
    oFile << "," << ImgAddress << "," << ImgName << "," << RoutineAddress << ","
    << RoutineName << "," << Count << endl;
  }
  //oFile << '"' << it_CurrentName->second << '"' << " icount " << it_CurrentNumber->second << endl;
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
