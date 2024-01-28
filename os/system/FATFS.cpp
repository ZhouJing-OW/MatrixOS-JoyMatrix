#include "MatrixOS.h"
#include "Device.h"
#include "esp_vfs_fat.h"
#include "esp_partition.h"
#include <fstream>
#include <list>
#include <queue>

#define AUTO_SAVE_INTERVAL 10000
namespace MatrixOS::FATFS 
{
  StaticTimer_t FATFS_autosave_timer_def;
  TimerHandle_t FATFS_autosave_timer;

  string nameInManage;
  string SuffixInManage;
  std::list<SaveVarInfo>* listInManage = nullptr;
  std::queue<std::pair<void*, uint16_t>> changedVar;
  
  inline string NameToPath(string appName, string suffix) { return "/storage/" + appName + "." + suffix; }

  void Scan()
  {
    if (changedVar.empty() || listInManage == nullptr) return;

    std::fstream fio;
    if (OpenFile(nameInManage, SuffixInManage, fio))
    {
      while(!changedVar.empty())                                                                                                                                                                                    
      {
        for (auto it = listInManage->begin(); it != listInManage->end(); it++)
        {
          if (*it->ptr == changedVar.front().first)
          {
            ListSavePart(*it, changedVar.front().second, fio);
            break; 
          }
        }
        changedVar.pop();
      }
    }
    fio.close();
  }

  char* LoadFile(size_t size, uint16_t& count, string appName, string suffix)
  {
    std::fstream fio;
    char* rtn;
    if(OpenFile(appName, suffix, fio, false)) 
    {
      fio.seekg(0, std::ios::end);
      count = fio.tellg() / size;
      rtn = (char*)heap_caps_malloc(fio.tellg(), MALLOC_CAP_SPIRAM);
      if (rtn == nullptr)
        return nullptr;
      fio.seekg(0, std::ios::beg);
      fio.read(rtn, size * count);
      fio.close(); //close file
      string log = "FATFS: " + appName + "." + suffix + " loaded";
      MLOGD(log, "%d variables", count);
      return rtn;
    }
    return nullptr;
  }

  bool OpenFile(string name, string suffix, std::fstream& fio, bool trunc)
  {
    string path = NameToPath(name, suffix);
    if (trunc)
      fio.open(path, std::ios::in | std::ios:: out | std::ios::binary | std::ios::trunc);
    else
      fio.open(path, std::ios::in | std::ios:: out | std::ios::binary);
    if (!fio.is_open()) {
      std::fstream fout;
      fout.open(path, std::ios::out | std::ios::binary);
      if (fout.is_open())
      {
        fout.close();
        return OpenFile(name, suffix, fio, false);
      }
      MLOGE("FATFS: Failed to open File. path", path );
      return false;
    }
    return true;
  }

  bool ListSave(string name, string suffix, std::list<SaveVarInfo>& varList, bool trunc)
  {
    std::fstream fio;
    if (OpenFile(name, suffix, fio, trunc))
    {
      for (auto it = varList.begin(); it != varList.end(); it++)
      {
        size_t size = it->size * it->count;
        fio.write((char*)*it->ptr, size);
      }
      fio.close();
      return true;
    }
    else
      return false;
  }

  bool ListLoad(string name, string suffix, std::list<SaveVarInfo>& varList)
  {
    std::fstream fio;
    if (OpenFile(name, suffix, fio, false))
    {
      for (auto it = varList.begin(); it != varList.end(); it++)
      {
        size_t size = it->size * it->count;
        *it->ptr = (void*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (*it->ptr == nullptr)
        {
          MLOGE("FATFS: ListLoad", "Failed to allocate memory");
          for (auto itFree = varList.begin(); itFree != it; ++itFree)
            {
              free(*itFree->ptr);
              *itFree->ptr = nullptr;
            }
            fio.close();
            return false;
        }
        it->tellp = fio.tellg();
        fio.read((char*)*it->ptr, size);
      }
      fio.close();
      return true;
    }
    else
      return false;
  }

  void SavePart(void* VariablePtr, size_t size, uint16_t pos, std::fstream& fio)
  { 
    fio.seekp(pos * size, std::ios::beg);
    fio.write((char*)VariablePtr, size);
  }

  void ListSavePart(SaveVarInfo& saveVar, uint16_t pos, std::fstream& fio)
  {
    fio.seekp(pos * saveVar.size + saveVar.tellp, std::ios::beg);
    fio.write((char*)*saveVar.ptr + pos * saveVar.size, saveVar.size);
  }

  void SaveContinuous(void* VariablePtr, size_t size, std::fstream& fio) { fio.write((char*)VariablePtr, size); }

  void MarkChanged(void* varPtr, uint16_t pose) { changedVar.push(std::pair{varPtr, pose}); }

  void VarManager(string name, string suffix, std::list<SaveVarInfo>& varList)
  {
    nameInManage = name;
    SuffixInManage = suffix;
    listInManage = &varList;
    ListLoad(name, suffix, varList);

    FATFS_autosave_timer =
    xTimerCreateStatic(NULL, AUTO_SAVE_INTERVAL, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(Scan), &FATFS_autosave_timer_def);
    xTimerStart(FATFS_autosave_timer, 0);
    MLOGD("FATFS", "Variable Manage Started");
  }

  void VarManageEnd()
  {
    if(!changedVar.empty())
      Scan();
    nameInManage = "";
    SuffixInManage = "";
    
    if(listInManage != nullptr)
    {
      for(auto it = listInManage->begin(); it != listInManage->end(); it++)
      {
        free(*it->ptr);
        *it->ptr = nullptr;
      }
      listInManage = nullptr;
    }
    xTimerStop(FATFS_autosave_timer, 0);
    MLOGD("FATFS", "Variable Manage Ended");
  }
}