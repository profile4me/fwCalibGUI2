OBJ=./build
LIBSSH_INC=-I/u/dborisen/packages/include 
LIBSSH_LIBS=-L/u/dborisen/packages/lib -lssh
CC=g++ `root-config --libs --cflags` ${LIBSSH_INC} ${LIBSSH_LIBS} -I${OBJ} -I${OBJ}/gui -I. -std=c++11 -g 
CC_GUI=g++ `root-config --glibs --cflags` ${LIBSSH_INC} ${LIBSSH_LIBS} -I${OBJ} -I. -std=c++11 -g 
HYDRA_FLAGS=-I${HADDIR}/include -L${HADDIR}/lib -lHydra -lDst -lStart -lRich -lMdc -lTof -lShower -lEmc -lRpc -lWall -lPionTracker -lMdcTrackG -lMdcTrackD -lParticle -lOra
.PHONY: all

all: app analysisDST

# DayMeta ##################################################
${OBJ}/DayMetaDict.cxx: DayMeta.cxx DayMeta.h LinkDef.h
	rootcint -f ${OBJ}/DayMetaDict.cxx -c -p ${LIBSSH_INC} -I/usr/include DayMeta.h LinkDef.h

${OBJ}/DayMetaDict.cxx.o: ${OBJ}/DayMetaDict.cxx
	${CC} -o ${OBJ}/DayMetaDict.cxx.o -c ${OBJ}/DayMetaDict.cxx

${OBJ}/DayMeta.cxx.o: DayMeta.cxx DayMeta.h
	${CC} -o ${OBJ}/DayMeta.cxx.o -c DayMeta.cxx

# CalibItem ##################################################
${OBJ}/CalibItemDict.cxx: CalibItem.cxx CalibItem.h LinkDef.h
	rootcint -f ${OBJ}/CalibItemDict.cxx -c -p ${LIBSSH_INC} CalibItem.h LinkDef.h

${OBJ}/CalibItemDict.cxx.o: ${OBJ}/CalibItemDict.cxx
	${CC} -o ${OBJ}/CalibItemDict.cxx.o -c ${OBJ}/CalibItemDict.cxx

${OBJ}/CalibItem.cxx.o: CalibItem.cxx CalibItem.h
	${CC} -o ${OBJ}/CalibItem.cxx.o -c CalibItem.cxx

# MainFrame ##################################################
${OBJ}/gui/MainFrameDict.cxx: MainFrame.cxx MainFrame.h LinkDef.h
	rootcint -f ${OBJ}/gui/MainFrameDict.cxx -c -p ${LIBSSH_INC} MainFrame.h LinkDef.h

${OBJ}/gui/MainFrameDict.cxx.o: ${OBJ}/gui/MainFrameDict.cxx
	${CC_GUI} -o ${OBJ}/gui/MainFrameDict.cxx.o -c ${OBJ}/gui/MainFrameDict.cxx

${OBJ}/gui/MainFrame.cxx.o: MainFrame.cxx MainFrame.h
	${CC_GUI} -o ${OBJ}/gui/MainFrame.cxx.o -c MainFrame.cxx

# TestClass ##################################################
${OBJ}/TestClassDict.cxx: TestClass.h LinkDef.h
	rootcint -f ${OBJ}/TestClassDict.cxx -c -p ${LIBSSH_INC} TestClass.h LinkDef.h

${OBJ}/TestClassDict.cxx.o: ${OBJ}/TestClassDict.cxx
	${CC} -o ${OBJ}/TestClassDict.cxx.o -c ${OBJ}/TestClassDict.cxx

${OBJ}/TestClass.cxx.o: TestClass.cxx
	${CC} -o ${OBJ}/TestClass.cxx.o -c TestClass.cxx


obj: ${OBJ}/DayMetaDict.cxx.o ${OBJ}/DayMeta.cxx.o  ${OBJ}/CalibItemDict.cxx.o ${OBJ}/CalibItem.cxx.o ${OBJ}/gui/MainFrameDict.cxx.o ${OBJ}/gui/MainFrame.cxx.o ${OBJ}/TestClassDict.cxx.o ${OBJ}/TestClass.cxx.o

app: obj  main.cc 
	${CC} -lSpectrum -o app $(wildcard ${OBJ}/*.o) main.cc 

analysisDST: analysisDST.cc 
	${CC} ${HYDRA_FLAGS} -o analysisDST analysisDST.cc

genTree: genTree.cc
	${CC} ${HYDRA_FLAGS} -o genTree genTree.cc

gui: obj gui.cc
	${CC_GUI} -lSpectrum -o gui $(wildcard ${OBJ}/gui/*.o) $(wildcard ${OBJ}/*.o) gui.cc

test: obj test.cc
	${CC} -lSpectrum -o test   test.cc