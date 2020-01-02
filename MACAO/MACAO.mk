##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=MACAO
ConfigurationName      :=Debug
WorkspacePath          :=/scratch/simulation_thanghoang/MACAO
ProjectPath            :=/scratch/simulation_thanghoang/MACAO/MACAO
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Thang Hoang
Date                   :=02/01/20
CodeLitePath           :=/home/thanghoang/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
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
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -std=c++11 -Wall -libstdc++ -c -fPIC $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/Utils.cpp$(ObjectSuffix) $(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix) $(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix) $(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix) $(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix) $(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix) $(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix) $(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/Utils.cpp$(ObjectSuffix): Utils.cpp $(IntermediateDirectory)/Utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils.cpp$(DependSuffix): Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils.cpp$(DependSuffix) -MM Utils.cpp

$(IntermediateDirectory)/Utils.cpp$(PreprocessSuffix): Utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils.cpp$(PreprocessSuffix) Utils.cpp

$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix): ORAM.cpp $(IntermediateDirectory)/ORAM.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/ORAM.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ORAM.cpp$(DependSuffix): ORAM.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ORAM.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ORAM.cpp$(DependSuffix) -MM ORAM.cpp

$(IntermediateDirectory)/ORAM.cpp$(PreprocessSuffix): ORAM.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ORAM.cpp$(PreprocessSuffix) ORAM.cpp

$(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix): ServerKaryORAMC.cpp $(IntermediateDirectory)/ServerKaryORAMC.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/ServerKaryORAMC.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ServerKaryORAMC.cpp$(DependSuffix): ServerKaryORAMC.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ServerKaryORAMC.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ServerKaryORAMC.cpp$(DependSuffix) -MM ServerKaryORAMC.cpp

$(IntermediateDirectory)/ServerKaryORAMC.cpp$(PreprocessSuffix): ServerKaryORAMC.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ServerKaryORAMC.cpp$(PreprocessSuffix) ServerKaryORAMC.cpp

$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix): ServerBinaryORAMO.cpp $(IntermediateDirectory)/ServerBinaryORAMO.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/ServerBinaryORAMO.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(DependSuffix): ServerBinaryORAMO.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(DependSuffix) -MM ServerBinaryORAMO.cpp

$(IntermediateDirectory)/ServerBinaryORAMO.cpp$(PreprocessSuffix): ServerBinaryORAMO.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ServerBinaryORAMO.cpp$(PreprocessSuffix) ServerBinaryORAMO.cpp

$(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix): ServerORAM.cpp $(IntermediateDirectory)/ServerORAM.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/ServerORAM.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ServerORAM.cpp$(DependSuffix): ServerORAM.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ServerORAM.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ServerORAM.cpp$(DependSuffix) -MM ServerORAM.cpp

$(IntermediateDirectory)/ServerORAM.cpp$(PreprocessSuffix): ServerORAM.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ServerORAM.cpp$(PreprocessSuffix) ServerORAM.cpp

$(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix): ClientORAM.cpp $(IntermediateDirectory)/ClientORAM.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/ClientORAM.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ClientORAM.cpp$(DependSuffix): ClientORAM.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ClientORAM.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ClientORAM.cpp$(DependSuffix) -MM ClientORAM.cpp

$(IntermediateDirectory)/ClientORAM.cpp$(PreprocessSuffix): ClientORAM.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ClientORAM.cpp$(PreprocessSuffix) ClientORAM.cpp

$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix): ClientBinaryORAMO.cpp $(IntermediateDirectory)/ClientBinaryORAMO.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/ClientBinaryORAMO.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(DependSuffix): ClientBinaryORAMO.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(DependSuffix) -MM ClientBinaryORAMO.cpp

$(IntermediateDirectory)/ClientBinaryORAMO.cpp$(PreprocessSuffix): ClientBinaryORAMO.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ClientBinaryORAMO.cpp$(PreprocessSuffix) ClientBinaryORAMO.cpp

$(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix): ClientKaryORAMC.cpp $(IntermediateDirectory)/ClientKaryORAMC.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/scratch/simulation_thanghoang/MACAO/MACAO/ClientKaryORAMC.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ClientKaryORAMC.cpp$(DependSuffix): ClientKaryORAMC.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ClientKaryORAMC.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ClientKaryORAMC.cpp$(DependSuffix) -MM ClientKaryORAMC.cpp

$(IntermediateDirectory)/ClientKaryORAMC.cpp$(PreprocessSuffix): ClientKaryORAMC.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ClientKaryORAMC.cpp$(PreprocessSuffix) ClientKaryORAMC.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


