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

VOID docount(UINT32 _CNT, UINT32 _TMPID) { RTNTABLE[_TMPID] += _CNT; }

/* ===================================================================== */

VOID BasicBlock(TRACE trc , VOID *v)
{
	//BBL head
	BBL _BBL=TRACE_BblHead(trc); 
	while (BBL_Valid(_BBL)) //check all _BBL's
	{
		RTN CurrentRTN = RTN_FindByAddress(BBL_Address(_BBL));
		UINT32 CurrentId = RTN_Id(CurrentRTN);


		if(RTNTABLE.find(CurrentId) == RTNTABLE.end()) //not exist
			RTNTABLE.insert(pair<UINT32,UINT32>(CurrentId,0));  //create new one 

		// check is the element already exist in RTNNAMES map
		if(RTNNAMES.find(CurrentId) == RTNNAMES.end()) //not exist
			RTNNAMES.insert(pair<UINT32,string>(CurrentId,RTN_FindNameByAddress(BBL_Address(_BBL))));  	//create new one
			
		BBL_InsertCall(_BBL, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, BBL_NumIns(_BBL), IARG_UINT32, CurrentRTN, IARG_END);
		_BBL = BBL_Next(_BBL); //next BBL
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
