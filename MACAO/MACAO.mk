##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=MACAO
ConfigurationName      :=Debug
WorkspacePath          :=/home/sadman/MACAO
ProjectPath            :=/home/sadman/MACAO/MACAO
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Sadman Sakib
Date                   :=19/05/20
CodeLitePath           :=/home/sadman/.codelite
LinkerName             :=/usr/bin/g++-5
SharedObjectLinkerName :=/usr/bin/g++-5 -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="MACAO.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -lntl -lgmp -lzmq -lpthread -ltomcrypt 
IncludePath            :=  $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)/usr/local/lib 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++-5
CC       := /usr/bin/gcc-5
CXXFLAGS :=  -g -O1 -std=c++11 -Wall -libstdc++ -c -fPIC $(Preprocessors)
CFLAGS   :=  -g -O1 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix) $(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix) $(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix) $(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix) $(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix) $(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix) $(IntermediateDirectory)/Utils.cpp$(ObjectSuffix) $(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix): ORAM.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ORAM.cpp$(DependSuffix) -MM ORAM.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/ORAM.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ORAM.cpp$(PreprocessSuffix): ORAM.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ORAM.cpp$(PreprocessSuffix) ORAM.cpp

$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix): ClientBinaryORAMO.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(DependSuffix) -MM ClientBinaryORAMO.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/ClientBinaryORAMO.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(PreprocessSuffix): ClientBinaryORAMO.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ClientBinaryORAMO.cpp$(PreprocessSuffix) ClientBinaryORAMO.cpp

$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix): ServerBinaryORAMO.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(DependSuffix) -MM ServerBinaryORAMO.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/ServerBinaryORAMO.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(PreprocessSuffix): ServerBinaryORAMO.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ServerBinaryORAMO.cpp$(PreprocessSuffix) ServerBinaryORAMO.cpp

$(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix): ServerKaryORAMC.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ServerKaryORAMC.cpp$(DependSuffix) -MM ServerKaryORAMC.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/ServerKaryORAMC.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ServerKaryORAMC.cpp$(PreprocessSuffix): ServerKaryORAMC.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ServerKaryORAMC.cpp$(PreprocessSuffix) ServerKaryORAMC.cpp

$(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix): ClientORAM.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ClientORAM.cpp$(DependSuffix) -MM ClientORAM.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/ClientORAM.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ClientORAM.cpp$(PreprocessSuffix): ClientORAM.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ClientORAM.cpp$(PreprocessSuffix) ClientORAM.cpp

$(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix): ServerORAM.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ServerORAM.cpp$(DependSuffix) -MM ServerORAM.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/ServerORAM.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ServerORAM.cpp$(PreprocessSuffix): ServerORAM.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ServerORAM.cpp$(PreprocessSuffix) ServerORAM.cpp

$(IntermediateDirectory)/Utils.cpp$(ObjectSuffix): Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils.cpp$(DependSuffix) -MM Utils.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils.cpp$(PreprocessSuffix): Utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils.cpp$(PreprocessSuffix) Utils.cpp

$(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix): ClientKaryORAMC.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ClientKaryORAMC.cpp$(DependSuffix) -MM ClientKaryORAMC.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sadman/MACAO/MACAO/ClientKaryORAMC.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ClientKaryORAMC.cpp$(PreprocessSuffix): ClientKaryORAMC.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ClientKaryORAMC.cpp$(PreprocessSuffix) ClientKaryORAMC.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


