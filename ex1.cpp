#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>
#include <string>

//#include <tuple>

using namespace::std;

/* ===================================================================== */
/* Global Variables                                                      */
/* ===================================================================== */

//Using 2 map objects, for name and counts
map <UINT32, UINT32> RTNTABLE;
map <UINT32, string> RTNNAMES;

class RoutineData {
public:
  /*RTN Routine;
  IMG Img;*/
  UINT32 Count;
  ADDRINT _RTN_ADDRESS;
  string _RTN_NAME;
  ADDRINT _IMG_ADDRESS;
  //string _IMG_NAME;
  /*RoutineData(RTN rtn, IMG img)
    : Routine(rtn), Img(img), Count(0) {}*/
  RoutineData(ADDRINT rtnAddress,string rtnName, ADDRINT imgAddress)
    : Count(0), _RTN_ADDRESS(rtnAddress), _RTN_NAME(rtnName),
      _IMG_ADDRESS(imgAddress) {}
};

//RoutineData rtnData;
map <UINT32, RoutineData> RoutineMap;

/*map <UINT32, UINT32> countMap;
map <UINT32, ADDRINT> rtnAddresses;
map <UINT32, string> rtnNames;
map <UINT32, ADDRINT> imgAdresses;
map <UINT32, string> imgNames;
map <UINT32, const string&> imgNames2;*/



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

//VOID PIN_FAST_ANALYSIS_CALL docount(UINT32 count, UINT32 RoutineID) { RTNTABLE[RoutineID] += count; }
VOID docount(UINT32 count, UINT32 RoutineID) { ((RoutineMap.find(RoutineID))->second).Count += count; }

/* ===================================================================== */

VOID BasicBlock(TRACE trc , VOID *v)
{
	//BBL head
	for (BBL bbl =TRACE_BblHead(trc); BBL_Valid(bbl); bbl = BBL_Next(bbl))
	{
		RTN CurrentRTN = RTN_FindByAddress(BBL_Address(bbl));
		UINT32 CurrentId = RTN_Id(CurrentRTN);

    ADDRINT RoutineAddress = RTN_Address(CurrentRTN);
    string RoutineName = RTN_FindNameByAddress(BBL_Address(bbl));

    IMG CurrentImg = IMG_FindByAddress(BBL_Address(bbl));

    //const string& tmp = IMG_Name(CurrentImg);
    //cout << "tmp: " << &tmp << endl;
    /*string ImgName;
    for (unsigned int i = 0; i < tmp.length(); i++) {
      ImgName.insert(*(tmp + i);
    }*/
    //string ImgName(tmp);
    //strcpy(&tmp, ImgName);
    //strcpy(IMG_Name(CurrentImg), ImgName);

    ADDRINT ImgAddress = IMG_Entry(CurrentImg);

    //if (countMap.find(CurrentId) == countMap.end()) {
    if (RoutineMap.find(CurrentId) == RoutineMap.end()) {
        RoutineData rtnData(RoutineAddress, RoutineName, ImgAddress);
        RoutineMap.insert(pair<UINT32,RoutineData>(CurrentId,rtnData));

        //RoutineData rtnData(CurrentRTN, CurrentImg);
        //RoutineMap.insert(pair<UINT32,RoutineData>(CurrentId,rtnData));

        /*countMap.insert(pair<UINT32,UINT32>(CurrentId,0));
        rtnAddresses.insert(pair<UINT32,ADDRINT>(CurrentId,RoutineAddress));
        rtnNames.insert(pair<UINT32,string>(CurrentId,RoutineName));
        imgAdresses.insert(pair<UINT32,ADDRINT>(CurrentId,ImgAddress));*/
        //imgNames.insert(pair<UINT32,string>(CurrentId,ImgName));
        //imgNames2.insert(pair<UINT32,const string&>(CurrentId,tmp));

        RTNTABLE.insert(pair<UINT32,UINT32>(CurrentId,0));  //create new one
    }
		//if(RTNTABLE.find(CurrentId) == RTNTABLE.end()) //not exist
			//RTNTABLE.insert(pair<UINT32,UINT32>(CurrentId,0));  //create new one

		// check is the element already exist in RTNNAMES map
		if(RTNNAMES.find(CurrentId) == RTNNAMES.end()) //not exist
			RTNNAMES.insert(pair<UINT32,string>(CurrentId,RTN_FindNameByAddress(BBL_Address(bbl))));  	//create new one

		BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, BBL_NumIns(bbl), IARG_UINT32, CurrentRTN, IARG_END);
	}
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
	//open file
	ofstream oFile;
	oFile.open("rtn-output.csv");
  //map <UINT32,UINT32> MaxMap;
  map <UINT32,RoutineData> MaxMap;
  //oFile << '"' << it_CurrentName->second << '"' << " icount " << it_CurrentNumber->second << endl;
  for (map<UINT32,RoutineData>::iterator iter = RoutineMap.begin(); iter != RoutineMap.end(); ++iter) {
    UINT32 Count = (iter->second).Count;
    MaxMap.insert(pair<UINT32,RoutineData>(Count,iter->second));
  }
  for (map<UINT32,RoutineData>::reverse_iterator iter = MaxMap.rbegin(); iter != MaxMap.rend(); ++iter) {
    /*RTN Routine((iter->second).Routine);
    IMG Img((iter->second).Img);
    ADDRINT ImgAddress = IMG_Entry(Img);
    //string ImgName = IMG_Name(Img);
    ADDRINT RoutineAddress = RTN_Address(Routine);
    string RoutineName = RTN_Name(Routine);*/

    ADDRINT imgAddress = (iter->second)._IMG_ADDRESS;
    string ImgAddress = StringFromAddrint(imgAddress);
    ADDRINT routineAddress = (iter->second)._RTN_ADDRESS;
    string RoutineAddress = StringFromAddrint(routineAddress);
    string RoutineName = (iter->second)._RTN_NAME;
    UINT32 Count = iter->first;
    oFile << ImgAddress << "," << "," << RoutineAddress << ","
    << RoutineName << "," << Count << endl;
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

