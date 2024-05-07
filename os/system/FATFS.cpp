#include "MatrixOS.h"
#include "Device.h"
#include "esp_vfs_fat.h"
#include "esp_partition.h"
#include <fstream>
#include <list>
#include <queue>
#include <map>
#include <set>

#define AUTO_SAVE_INTERVAL 5000
namespace MatrixOS::FATFS 
{
  StaticTimer_t FATFS_autosave_timer_def;
  TimerHandle_t FATFS_autosave_timer;

  string nameInManage;
  std::list<std::pair<SaveVarInfo, string>> listInManage; // SaveVarInfo, suffix
  std::set<std::pair<void*, uint16_t>> changedVar;
  std::set<string> suffixList;
  
  inline string NameToPath(string appName, string suffix) { return "/storage/" + appName + "." + suffix; }

  void Scan()
  {
    if (MatrixOS::MidiCenter::TransState() && MatrixOS::MidiCenter::TransState()->play) return;
    if (changedVar.empty() || listInManage.empty()) return;
    string suffix = "";
    SaveVarInfo saveInfo;
    uint16_t savePos = 0;

    for (auto it : listInManage) {
      if (it.first.ptr != nullptr && *it.first.ptr == changedVar.begin()->first) {
        suffix = it.second;
        saveInfo = it.first; 
        savePos = changedVar.begin()->second;
        break;
      }
    }
    changedVar.erase(changedVar.begin());

    if(suffix == "") return;

    std::fstream fio;
    if (OpenFile(nameInManage, suffix, fio, false))
    {
      ListSavePart(saveInfo, savePos, fio);
      MLOGD("FATFS: " + nameInManage + "." + suffix, "Variable Pos %d Saved.", fio.tellp());
      fio.close();
    }

  }

  //   void Scan()
  // {
  //   if (changedVar.empty() || listInManage.empty()) return;
  //   std::map<string, std::queue<std::pair<SaveVarInfo, uint16_t>>> saveMap;

  //   while (!changedVar.empty())
  //   {
  //     for (auto it : listInManage) {
  //       if (it.first.ptr != nullptr && *it.first.ptr == changedVar.begin()->first) {
  //         saveMap[it.second].push({it.first, changedVar.begin()->second});
  //         break;
  //       }
  //     }
  //     changedVar.erase(changedVar.begin());
  //   }
  //   if(saveMap.empty()) return;

  //   for (auto it : saveMap)
  //   {
  //     std::fstream fio;
  //     if (OpenFile(nameInManage, it.first, fio, false))
  //     {
  //       while (!it.second.empty())
  //       {
  //         auto itVar = it.second.front();
  //           ListSavePart(itVar.first, itVar.second, fio);
  //           MLOGD("FATFS: " + nameInManage + "." + it.first, "Variable Pos %d Saved.", fio.tellp());
  //         it.second.pop();
  //       }
  //       fio.close();
  //     }
  //   }
  // }

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
      {
        MLOGE("FATFS: LoadFile", "Failed to allocate memory");
        fio.close();
        return nullptr;
      }
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
        it->filePos = fio.tellp();
        fio.write((char*)*it->ptr, size);
      }
      
      MLOGD("FATFS: " + name + "." + suffix, "Variable Saved. size = %d", fio.tellp());
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
      size_t checkSize = 0;
      fio.seekg(0, std::ios::end);

      if (fio.tellg() == 0)
      {
        MLOGE("FATFS: ListLoad", name + "." + suffix + "File is empty");
        fio.close();
        return false;
      }

      for (auto it = varList.begin(); it != varList.end(); it++)
        checkSize += it->size * it->count;
      
      if (fio.tellg() != checkSize)
      {
        MLOGE("FATFS: ListLoad", name + "." + suffix + "File size does not match");
        fio.close();
        return false;
      }
      fio.seekg(0, std::ios::beg);

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
        it->filePos = fio.tellg();
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
    fio.seekp(pos * saveVar.size + saveVar.filePos, std::ios::beg);
    fio.write((char*)*saveVar.ptr + pos * saveVar.size, saveVar.size);
  }

  void SaveContinuous(void* VariablePtr, size_t size, std::fstream& fio) { fio.write((char*)VariablePtr, size); }

  void MarkChanged(void* varPtr, uint16_t pose) { changedVar.insert({varPtr, pose});
  }

  bool VarManager(string name, string suffix, std::list<SaveVarInfo>& varList)
  {
    if (nameInManage == "")
    {
      FATFS_autosave_timer =
      xTimerCreateStatic(NULL, AUTO_SAVE_INTERVAL, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(Scan), &FATFS_autosave_timer_def);
      xTimerStart(FATFS_autosave_timer, 0);
      nameInManage = name;
    }

    if (nameInManage != name)
    {
      MLOGE("FATFS: VarManager", "Name does not match");
      return false;
    }

    if(suffixList.find(suffix) != suffixList.end()) {
      MLOGE("FATFS: VarManager", "Suffix ." + suffix +" already exists");
    }
    
    bool rtn = ListLoad(name, suffix, varList);
        
    if (rtn)
    {
      MLOGD("FATFS: " + name + "." + suffix, "Variable Loaded");
      for(auto it = varList.begin(); it != varList.end(); it++)
        listInManage.push_back({*it, suffix});
      suffixList.insert(suffix);
    }
    else
      MLOGE("FATFS: " + name + "." + suffix, "Variable Load Failed");
    return rtn;
  }

  void VarManageEnd(string suffix)
  {
    while(!changedVar.empty()) { Scan();}

    for(auto it = listInManage.begin(); it != listInManage.end();)
    {
      if(it->second == suffix && it->first.ptr != nullptr && *it->first.ptr != nullptr)
      {
        free(*it->first.ptr);
        *it->first.ptr = nullptr;
        it = listInManage.erase(it);
      }
      else
        it++;
    }
    
    suffixList.erase(suffix);

    if(listInManage.empty())
    {
      MLOGD("FATFS", nameInManage + " Variable Manage Ended");
      nameInManage = "";
      xTimerStop(FATFS_autosave_timer, 0);
    }
  }
}