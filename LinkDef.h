#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#ifdef __TestClass__
	#pragma link C++ class TestClass;
#endif

#ifdef __MainFrame__
	#pragma link C++ class MainFrame;
#endif

#if defined(__CalibItem__) && !defined(__MainFrame__)
	#pragma link C++ class CalibItem+;
#endif


#if defined(__DayMeta__) && !defined(__CalibItem__) 
	#pragma link C++ class DayMeta+;
#endif

#endif
