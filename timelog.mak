# Microsoft Visual C++ generated build script - Do not modify

PROJ = TIMELOG
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG
R_RCDEFINES = /dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = E:\MARTY\WINDEV\TIMELOG\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = TIMELOG.C   
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W4 /Zi /AM /Od /D "_DEBUG" /FR /GA /Fd"TIMELOG.PDB"
CFLAGS_R_WEXE = /nologo /W3 /AM /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE 
LIBS_D_WEXE = oldnames libw mlibcew commdlg.lib 
LIBS_R_WEXE = oldnames libw mlibcew 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = TIMELOG.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = TIMELOG.SBR \
		TIMEUTIL.SBR \
		SCANLOG.SBR \
		LOGFILE.SBR \
		PROJECT.SBR


TIMELOG_DEP = e:\marty\windev\timelog\wininc.h \
	e:\marty\windev\timelog\tldlg.h \
	e:\marty\windev\timelog\timelog.h \
	e:\marty\windev\timelog\project.h \
	e:\marty\windev\timelog\tloghlp.h \
	e:\marty\windev\timelog\help/timelog.map \
	e:\marty\windev\timelog\scanlog.h \
	e:\marty\windev\timelog\logfile.h \
	e:\marty\windev\timelog\timeutil.h


TIMELOG_RCDEP = e:\marty\windev\timelog\iconlog.ico


TIMEUTIL_DEP = e:\marty\windev\timelog\timeutil.h


SCANLOG_DEP = e:\marty\windev\timelog\wininc.h \
	e:\marty\windev\timelog\logfile.h


LOGFILE_DEP = e:\marty\windev\timelog\logfile.h


PROJECT_DEP = e:\marty\windev\timelog\wininc.h \
	e:\marty\windev\timelog\project.h


all:	$(PROJ).EXE $(PROJ).BSC

TIMELOG.OBJ:	TIMELOG.C $(TIMELOG_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c TIMELOG.C

TIMELOG.RES:	TIMELOG.RC $(TIMELOG_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r TIMELOG.RC

TIMEUTIL.OBJ:	TIMEUTIL.C $(TIMEUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TIMEUTIL.C

SCANLOG.OBJ:	SCANLOG.C $(SCANLOG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SCANLOG.C

LOGFILE.OBJ:	LOGFILE.C $(LOGFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LOGFILE.C

PROJECT.OBJ:	PROJECT.C $(PROJECT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PROJECT.C


$(PROJ).EXE::	TIMELOG.RES

$(PROJ).EXE::	TIMELOG.OBJ TIMEUTIL.OBJ SCANLOG.OBJ LOGFILE.OBJ PROJECT.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
TIMELOG.OBJ +
TIMEUTIL.OBJ +
SCANLOG.OBJ +
LOGFILE.OBJ +
PROJECT.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
d:\msvc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) TIMELOG.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	TIMELOG.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) TIMELOG.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
