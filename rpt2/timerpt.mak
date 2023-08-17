# Microsoft Visual C++ generated build script - Do not modify

PROJ = TIMERPT
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG
R_RCDEFINES = /dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = E:\MARTY\WINDEV\TIMELOG\RPT2\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = LOGRPT.C    
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Od /D "_DEBUG" /I ".." /FR /GA /Fd"TLRPT.PDB"
CFLAGS_R_WEXE = /nologo /W3 /Zd /AL /D "NDEBUG" /I ".." /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /MAP /LINE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = TIMERPT.DEF
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
SBRS = LOGRPT.SBR \
		LOGFILE.SBR \
		TIMEUTIL.SBR \
		PROJECT.SBR \
		TLRPTDLG.SBR \
		TLRPTTS.SBR \
		TLRPTDSP.SBR \
		TLRPTVAR.SBR \
		TIMERPT.SBR \
		TLRPTHLP.SBR \
		TLRPTABT.SBR


LOGRPT_DEP = e:\marty\windev\timelog\rpt2\logrpt.h


LOGFILE_DEP = e:\marty\windev\timelog\logfile.h


TIMEUTIL_DEP = e:\marty\windev\timelog\timeutil.h


PROJECT_DEP = e:\marty\windev\timelog\wininc.h \
	e:\marty\windev\timelog\project.h


TIMERPT_RCDEP = e:\marty\windev\timelog\rpt2\iconlog.ico


TLRPTDLG_DEP = e:\marty\windev\timelog\rpt2\tlrptdlg.h \
	e:\marty\windev\timelog\rpt2\timerpt.h \
	e:\marty\windev\timelog\rpt2\logrpt.h \
	e:\marty\windev\timelog\rpt2\tlrptdsp.h \
	e:\marty\windev\timelog\rpt2\tlrpthlp.h \
	e:\marty\windev\timelog\rpt2\help/timerpt.map \
	e:\marty\windev\timelog\rpt2\tlrptvar.h \
	e:\marty\windev\timelog\rpt2\tlrptts.h \
	e:\marty\windev\timelog\rpt2\tlrptabt.h


TLRPTTS_DEP = e:\marty\windev\timelog\rpt2\tlrptts.h \
	e:\marty\windev\timelog\rpt2\tlrptdlg.h


TLRPTDSP_DEP = e:\marty\windev\timelog\rpt2\tlrptdsp.h \
	e:\marty\windev\timelog\rpt2\timerpt.h \
	e:\marty\windev\timelog\rpt2\logrpt.h \
	e:\marty\windev\timelog\rpt2\tlrpthlp.h \
	e:\marty\windev\timelog\rpt2\help/timerpt.map


TLRPTVAR_DEP = e:\marty\windev\timelog\rpt2\tlrptvar.h \
	e:\marty\windev\timelog\rpt2\tlrptdlg.h


TIMERPT_DEP = e:\marty\windev\timelog\rpt2\timerpt.h \
	e:\marty\windev\timelog\rpt2\logrpt.h \
	e:\marty\windev\timelog\rpt2\tlrptdlg.h \
	e:\marty\windev\timelog\rpt2\tlrptdsp.h \
	e:\marty\windev\timelog\rpt2\tlrpthlp.h \
	e:\marty\windev\timelog\rpt2\help/timerpt.map


TLRPTHLP_DEP = e:\marty\windev\timelog\rpt2\tlrpthlp.h \
	e:\marty\windev\timelog\rpt2\help/timerpt.map


TLRPTABT_DEP = e:\marty\windev\timelog\rpt2\tlrptabt.h \
	e:\marty\windev\timelog\rpt2\timerpt.h \
	e:\marty\windev\timelog\rpt2\tlrpthlp.h \
	e:\marty\windev\timelog\rpt2\help/timerpt.map


all:	$(PROJ).EXE $(PROJ).BSC

LOGRPT.OBJ:	LOGRPT.C $(LOGRPT_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c LOGRPT.C

LOGFILE.OBJ:	..\LOGFILE.C $(LOGFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\LOGFILE.C

TIMEUTIL.OBJ:	..\TIMEUTIL.C $(TIMEUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\TIMEUTIL.C

PROJECT.OBJ:	..\PROJECT.C $(PROJECT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\PROJECT.C

TIMERPT.RES:	TIMERPT.RC $(TIMERPT_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r TIMERPT.RC

TLRPTDLG.OBJ:	TLRPTDLG.C $(TLRPTDLG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TLRPTDLG.C

TLRPTTS.OBJ:	TLRPTTS.C $(TLRPTTS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TLRPTTS.C

TLRPTDSP.OBJ:	TLRPTDSP.C $(TLRPTDSP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TLRPTDSP.C

TLRPTVAR.OBJ:	TLRPTVAR.C $(TLRPTVAR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TLRPTVAR.C

TIMERPT.OBJ:	TIMERPT.C $(TIMERPT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TIMERPT.C

TLRPTHLP.OBJ:	TLRPTHLP.C $(TLRPTHLP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TLRPTHLP.C

TLRPTABT.OBJ:	TLRPTABT.C $(TLRPTABT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TLRPTABT.C


$(PROJ).EXE::	TIMERPT.RES

$(PROJ).EXE::	LOGRPT.OBJ LOGFILE.OBJ TIMEUTIL.OBJ PROJECT.OBJ TLRPTDLG.OBJ TLRPTTS.OBJ \
	TLRPTDSP.OBJ TLRPTVAR.OBJ TIMERPT.OBJ TLRPTHLP.OBJ TLRPTABT.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
LOGRPT.OBJ +
LOGFILE.OBJ +
TIMEUTIL.OBJ +
PROJECT.OBJ +
TLRPTDLG.OBJ +
TLRPTTS.OBJ +
TLRPTDSP.OBJ +
TLRPTVAR.OBJ +
TIMERPT.OBJ +
TLRPTHLP.OBJ +
TLRPTABT.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
d:\msvc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) TIMERPT.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	TIMERPT.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) TIMERPT.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
