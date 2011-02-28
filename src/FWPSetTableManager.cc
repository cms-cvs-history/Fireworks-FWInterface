// -*- C++ -*-
//
// Package:     FWInterface
// Class  :     FWPSetTableManager
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  
//         Created:  Mon Feb 28 17:06:54 CET 2011
// $Id: FWPSetTableManager.cc,v 1.1 2011/02/28 18:47:35 amraktad Exp $
//

#include <iostream>
#include <sstream>
#include <cstring>
#include <map>

#include "Fireworks/FWInterface/src/FWPSetTableManager.h"
#include "Fireworks/TableWidget/src/FWTabularWidget.h"
#include "Fireworks/TableWidget/interface/GlobalContexts.h"

#include "FWCore/Framework/interface/ScheduleInfo.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Parse.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "TGTextEntry.h"


// FIXME: copied from Entry.cc should find a way to use the original
//        table.
struct TypeTrans {
   TypeTrans();

   typedef std::vector<std::string> CodeMap;
   CodeMap table_;
   std::map<std::string, char> type2Code_;
};

TypeTrans::TypeTrans():table_(255) {
   table_['b'] = "vBool";
   table_['B'] = "bool";
   table_['i'] = "vint32";
   table_['I'] = "int32";
   table_['u'] = "vuint32";
   table_['U'] = "uint32";
   table_['l'] = "vint64";
   table_['L'] = "int64";
   table_['x'] = "vuint64";
   table_['X'] = "uint64";
   table_['s'] = "vstring";
   table_['S'] = "string";
   table_['d'] = "vdouble";
   table_['D'] = "double";
   table_['p'] = "vPSet";
   table_['P'] = "PSet";
   table_['T'] = "path";
   table_['F'] = "FileInPath";
   table_['t'] = "InputTag";
   table_['v'] = "VInputTag";
   table_['g'] = "ESInputTag";
   table_['G'] = "VESInputTag";
   table_['e'] = "VEventID";
   table_['E'] = "EventID";
   table_['m'] = "VLuminosityBlockID";
   table_['M'] = "LuminosityBlockID";
   table_['a'] = "VLuminosityBlockRange";
   table_['A'] = "LuminosityBlockRange";
   table_['r'] = "VEventRange";
   table_['R'] = "EventRange";

   for(CodeMap::const_iterator itCode = table_.begin(), itCodeEnd = table_.end();
       itCode != itCodeEnd;
       ++itCode) {
      type2Code_[*itCode] = (itCode - table_.begin());
   }
}

static TypeTrans const sTypeTranslations;


//==============================================================================
//==============================================================================
//================== ENTRY EDITOR ==============================================
//==============================================================================
//==============================================================================


template <class T>
bool editNumericParameter(edm::ParameterSet &ps, bool tracked, 
                          const std::string &label, 
                          const std::string &value) 
{
   std::stringstream  str(value);
   T v;
   str >> v;
   bool fail = str.fail();
   if (tracked)
      ps.addParameter(label, v);
   else
      ps.addUntrackedParameter(label, v);
         
   return fail;
}

void editStringParameter(edm::ParameterSet &ps, bool tracked,
                         const std::string &label,
                         const std::string &value)
{
   if (tracked)
      ps.addParameter(label, value);
   else
      ps.addUntrackedParameter(label, value);
}

void editFileInPath(edm::ParameterSet &ps, bool tracked,
                    const std::string &label,
                    const std::string &value)
{
   if (tracked)
      ps.addParameter(label, edm::FileInPath(value));
   else
      ps.addUntrackedParameter(label, edm::FileInPath(value));
}

bool editVInputTag(edm::ParameterSet &ps, bool tracked,
                   const std::string &label,
                   const std::string &value)
{ 
   std::vector<edm::InputTag> inputTags;
   std::stringstream iss(value);
   std::string vitem;
   bool fail = false;
   size_t fst, lst;

   while (getline(iss, vitem, ','))
   {
      fst = vitem.find("[");
      lst = vitem.find("]");
        
      if ( fst != std::string::npos )
         vitem.erase(fst,1);
      if ( lst != std::string::npos )
         vitem.erase(lst,1);
        
      std::vector<std::string> tokens = edm::tokenize(vitem, ":");
      size_t nwords = tokens.size();
        
      if ( nwords > 3 )
      {
         fail = true;
         return fail;
      }
      else 
      {
         std::string it_label("");
         std::string it_instance("");
         std::string it_process("");

         if ( nwords > 0 ) 
            it_label = tokens[0];
         if ( nwords > 1 ) 
            it_instance = tokens[1];
         if ( nwords > 2 ) 
            it_process  = tokens[2];
        
         inputTags.push_back(edm::InputTag(it_label, it_instance, it_process));
      }
   }
     
   if (tracked)
      ps.addParameter(label, inputTags);
   else
      ps.addUntrackedParameter(label, inputTags);

   return fail;
}
  

bool editInputTag(edm::ParameterSet &ps, bool tracked,
                  const std::string &label,
                  const std::string &value)
{
   std::vector<std::string> tokens = edm::tokenize(value, ":");
   size_t nwords = tokens.size();
     
   bool fail;

   if ( nwords > 3 ) 
   {
      fail = true;
   }
   else
   {           
      std::string it_label("");
      std::string it_instance("");
      std::string it_process("");

      if ( nwords > 0 ) 
         it_label = tokens[0];
      if ( nwords > 1 ) 
         it_instance = tokens[1];
      if ( nwords > 2 ) 
         it_process  = tokens[2];

      if ( tracked )
         ps.addParameter(label, edm::InputTag(it_label, it_instance, it_process));
      else
         ps.addUntrackedParameter(label, edm::InputTag(it_label, it_instance, it_process));
            
      fail = false;
   }
           
   return fail;
}

bool editESInputTag(edm::ParameterSet &ps, bool tracked,
                    const std::string &label,
                    const std::string &value)
{
   std::vector<std::string> tokens = edm::tokenize(value, ":");
   size_t nwords = tokens.size();
      
   bool fail;
  
   if ( nwords > 2 )
   {
      fail = true;    
   }
   else
   {             
      std::string it_module("");
      std::string it_data("");

      if ( nwords > 0 ) 
         it_module = tokens[0];
      if ( nwords > 1 ) 
         it_data = tokens[1];

      if ( tracked )
         ps.addParameter(label, edm::ESInputTag(it_module, it_data));
      else
         ps.addUntrackedParameter(label, edm::ESInputTag(it_module, it_data));
        
      fail = false;
   }

   return fail;
}
  
template <typename T>
void editVectorParameter(edm::ParameterSet &ps, bool tracked,
                         const std::string &label,
                         const std::string &value)
{
   std::vector<T> valueVector;
      
   std::stringstream iss(value);
   std::string vitem;
      
   size_t fst, lst;

   while (getline(iss, vitem, ','))
   {
      fst = vitem.find("[");
      lst = vitem.find("]");
        
      if ( fst != std::string::npos )
         vitem.erase(fst,1);
      if ( lst != std::string::npos )
         vitem.erase(lst,1);
        
      std::stringstream oss(vitem);
      T on;
      oss >> on;

      valueVector.push_back(on);
   }
     
   if (tracked)
      ps.addParameter(label, valueVector);
   else
      ps.addUntrackedParameter(label, valueVector);
}
  

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

void FWPSetTableManager::handlePSetEntry(const edm::ParameterSetEntry& entry, const std::string& key)
{
   FWPSetTableManager::PSetData data;
   data.label = key;
   data.tracked = entry.isTracked();
   data.level = m_parentStack.size();
   data.parent = m_parentStack.back();
   data.type = 'P';
   data.module = m_modules.size() - 1;
   data.path = m_paths.size() - 1;
   data.pset = entry.pset();
   data.editable = false;
   m_parentStack.push_back(m_entries.size());
   m_entries.push_back(data);

   handlePSet(entry.pset());
   m_parentStack.pop_back();
}

void FWPSetTableManager::handleVPSetEntry(const edm::VParameterSetEntry& entry, const std::string& key)                     
{
   FWPSetTableManager::PSetData data;
   data.label = key;
   data.tracked = entry.isTracked();
   data.level = m_parentStack.size();
   data.parent = m_parentStack.back();
   data.type = 'p';
   data.module = m_modules.size() - 1;
   data.path = m_paths.size() - 1;
   data.editable = false;
   m_parentStack.push_back(m_entries.size());
   m_entries.push_back(data);

   std::stringstream ss;

   for (size_t i = 0, e = entry.vpset().size(); i != e; ++i)
   {
      ss.str("");
      ss << key << "[" << i << "]";
      FWPSetTableManager::PSetData vdata;
      vdata.label = ss.str();
      vdata.tracked = entry.isTracked();
      vdata.level = m_parentStack.size();
      vdata.parent = m_parentStack.back();
      vdata.module = m_modules.size() - 1;
      vdata.path = m_paths.size() - 1;
      vdata.editable = false;
      m_parentStack.push_back(m_entries.size());
      m_entries.push_back(vdata);
      handlePSet(entry.vpset()[i]);
      m_parentStack.pop_back();
   }
   m_parentStack.pop_back();
}

void FWPSetTableManager::handlePSet(const edm::ParameterSet &ps)
{
   typedef edm::ParameterSet::table::const_iterator TIterator;
   for (TIterator i = ps.tbl().begin(), e = ps.tbl().end(); i != e; ++i)
      handleEntry(i->second, i->first);

   typedef edm::ParameterSet::psettable::const_iterator PSIterator;
   for (PSIterator i = ps.psetTable().begin(), e = ps.psetTable().end(); i != e; ++i)
      handlePSetEntry(i->second, i->first);

   typedef edm::ParameterSet::vpsettable::const_iterator VPSIterator;
   for (VPSIterator i = ps.vpsetTable().begin(), e = ps.vpsetTable().end(); i != e; ++i)
      handleVPSetEntry(i->second, i->first);
}
    
template <class T>
void FWPSetTableManager::createScalarString(PSetData &data, T v)
{
   std::stringstream ss;
   ss << v;
   data.value = ss.str();
   m_entries.push_back(data);
}

template <typename T>
void FWPSetTableManager::createVectorString(FWPSetTableManager::PSetData &data, const T &v, bool quotes)
{
   std::stringstream ss;
   ss << "[";
   for (size_t ii = 0, ie = v.size(); ii != ie; ++ii)
   {
      if (quotes)
         ss << "\""; 
      ss << v[ii];
      if (quotes)
         ss << "\"";
      if (ii + 1 != ie) 
         ss << ", ";
   }
   ss << "]";
   data.value = ss.str();
   m_entries.push_back(data);
}

void FWPSetTableManager::sortWithFilter(const char *filter)
{
   m_filter = filter;
   sort(-1, true);
   dataChanged();
}

void FWPSetTableManager::setCellValueEditor(TGTextEntry *editor)
{
   m_editor = editor;
   m_renderer.setCellEditor(editor);
}

void FWPSetTableManager::handleEntry(const edm::Entry &entry,const std::string &key)
{
   std::stringstream ss;
   FWPSetTableManager::PSetData data;
   data.label = key;
   data.tracked = entry.isTracked();
   data.type = entry.typeCode();
   data.level = m_parentStack.size();
   data.parent = m_parentStack.back();
   data.module = m_modules.size() - 1;
   data.type = entry.typeCode();
   if (data.label[0] == '@' || data.level > 2)
      data.editable = false;
   else
      data.editable = true;

   switch(entry.typeCode())
   {
      case 'b':
      {
         data.value = entry.getBool() ? "True" : "False";
         m_entries.push_back(data);
         break;
      }
      case 'B':
      {
         data.value = entry.getBool() ? "True" : "False";
         m_entries.push_back(data);
         break;
      }
      case 'i':
      {
         createVectorString(data, entry.getVInt32(), false);
         break;
      }
      case 'I':
      {
         createScalarString(data, entry.getInt32());
         break;
      }
      case 'u':
      {
         createVectorString(data, entry.getVUInt32(), false);
         break;
      }
      case 'U':
      {
         createScalarString(data, entry.getUInt32());
         break;
      }
      case 'l':
      {
         createVectorString(data, entry.getVInt64(), false);
         break;
      }
      case 'L':
      {
         createScalarString(data, entry.getInt32());
         break;
      }
      case 'x':
      {
         createVectorString(data, entry.getVUInt64(), false);
         break;
      }
      case 'X':
      {
         createScalarString(data, entry.getUInt64());
         break;
      }
      case 's':
      {
         createVectorString(data, entry.getVString(), false);
         break;
      }
      case 'S':
      {
         createScalarString(data, entry.getString());
         break;
      }
      case 'd':
      {
         createVectorString(data, entry.getVDouble(), false);
         break;
      }
      case 'D':
      { 
         createScalarString(data, entry.getDouble());
         break;
      }
      case 'p':
      {
         std::vector<edm::ParameterSet> psets = entry.getVPSet();
         for (size_t psi = 0, pse = psets.size(); psi != pse; ++psi)
            handlePSet(psets[psi]);
         break;
      }
      case 'P':
      {    
         handlePSet(entry.getPSet());
         break;
      }
      case 't':
      {
         data.value = entry.getInputTag().encode();
         m_entries.push_back(data);
         break;
      } 
      case 'v':
      {
         std::vector<std::string> tags;
         tags.resize(entry.getVInputTag().size());
         for (size_t iti = 0, ite = tags.size(); iti != ite; ++iti) 
            tags[iti] = entry.getVInputTag()[iti].encode();
         createVectorString(data, tags, true);
         break;
      }        
      case 'g':
      {
         data.value = entry.getESInputTag().encode();
         m_entries.push_back(data);
         break;
      }
      case 'G':
      {
         std::vector<std::string> tags;
         tags.resize(entry.getVESInputTag().size());
         for (size_t iti = 0, ite = tags.size(); iti != ite; ++iti) 
            tags[iti] = entry.getVESInputTag()[iti].encode();
         createVectorString(data, tags, true);
         break;
      }
      case 'F':
      {
         createScalarString(data, entry.getFileInPath().relativePath());
         break;
      }
      case 'e':
      {
         data.editable = false;
         std::vector<edm::EventID> ids;
         ids.resize(entry.getVEventID().size());
         for ( size_t iri = 0, ire = ids.size(); iri != ire; ++iri )
            ids[iri] = entry.getVEventID()[iri];
         createVectorString(data, ids, true);
         break;
      }
      case 'E':
      {
         data.editable = false;
         createScalarString(data, entry.getEventID());
         break;
      }
      case 'm':
      {
         data.editable = false;
         std::vector<edm::LuminosityBlockID> ids;
         ids.resize(entry.getVLuminosityBlockID().size());
         for ( size_t iri = 0, ire = ids.size(); iri != ire; ++iri )
            ids[iri] = entry.getVLuminosityBlockID()[iri];
         createVectorString(data, ids, true);
         break;
      }
      case 'M':
      {
         data.editable = false;
         createScalarString(data, entry.getLuminosityBlockID());
         break;
      }
      case 'a':
      {
         data.editable = false;
         std::vector<edm::LuminosityBlockRange> ranges;
         ranges.resize(entry.getVLuminosityBlockRange().size());
         for ( size_t iri = 0, ire = ranges.size(); iri != ire; ++iri )
            ranges[iri] = entry.getVLuminosityBlockRange()[iri];
         createVectorString(data, ranges, true);
         break;
      }
      case 'A':
      {
         data.editable = false;
         createScalarString(data, entry.getLuminosityBlockRange());
         break;
      }
      case 'r':
      {
         data.editable = false;
         std::vector<edm::EventRange> ranges;
         ranges.resize(entry.getVEventRange().size());
         for ( size_t iri = 0, ire = ranges.size(); iri != ire; ++iri )
            ranges[iri] = entry.getVEventRange()[iri];
         createVectorString(data, ranges, true);
         break;
      }
      case 'R':
      {
         data.editable = false;
         createScalarString(data, entry.getEventRange());
         break;          
      }
      default:
      {
         break;
      }
   }
}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//

FWPSetTableManager::FWPSetTableManager()
   : m_selectedRow(-1),
     m_greenGC(0),
     m_redGC(0),
     m_grayGC(0),
     m_bgGC(0)
{  
   m_boldRenderer.setGraphicsContext(&fireworks::boldGC());
   m_italicRenderer.setGraphicsContext(&fireworks::italicGC());

   m_greenGC = new TGGC(fireworks::boldGC());
   m_greenGC->SetForeground(gVirtualX->GetPixel(kGreen-5));

   m_redGC = new TGGC(fireworks::boldGC());
   m_redGC->SetForeground(gVirtualX->GetPixel(kRed-5));

   m_grayGC = new TGGC(fireworks::italicGC());
   m_grayGC->SetForeground(gVirtualX->GetPixel(kGray+1));

   m_bgGC = new TGGC(*gClient->GetResourcePool()->GetFrameGC());
   m_bgGC->SetBackground(gVirtualX->GetPixel(kGray));

  
   m_pathPassedRenderer.setGraphicsContext(m_greenGC);
   m_pathPassedRenderer.setHighlightContext(m_bgGC);
   m_pathPassedRenderer.setIsParent(true);

   m_pathFailedRenderer.setGraphicsContext(m_redGC);
   m_pathFailedRenderer.setHighlightContext(m_bgGC);
   m_pathFailedRenderer.setIsParent(true);
      
   m_editingDisabledRenderer.setGraphicsContext(m_grayGC);
   m_editingDisabledRenderer.setHighlightContext(m_bgGC);

   // Italic color doesn't seem to show up well event though
   // modules are displayed in italic
   m_modulePassedRenderer.setGraphicsContext(m_greenGC);
   m_modulePassedRenderer.setIsParent(true);
   m_moduleFailedRenderer.setGraphicsContext(m_redGC);
   m_moduleFailedRenderer.setIsParent(true);

   // Debug stuff to dump font list.
   //      std::cout << "Available fonts: " << std::endl;
   //      gClient->GetFontPool()->Print();
       
   reset();
}

FWPSetTableManager::~FWPSetTableManager()
{
}

void FWPSetTableManager::implSort(int, bool)
{
   // Decide whether or not items match the filter.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
   {
      PSetData &data = m_entries[i];
      // First of all decide whether or not we match
      // the filter.
      if (m_filter.empty())
         data.matches = false;
      else if (strstr(data.label.c_str(), m_filter.c_str()))
         data.matches = true;
      else
         data.matches = false;
   }

   // We reset whether or not a given parent has children that match the
   // filter, and we recompute the whole information by checking all the
   // children.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
      m_entries[i].childMatches = false;

   std::vector<int> stack;
   int previousLevel = 0;
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
   {
      PSetData &data = m_entries[i];
      // Top level.
      if (data.parent == (size_t)-1)
      {
         previousLevel = 0;
         continue;
      }
      // If the level is greater than the previous one,
      // it means we are among the children of the 
      // previous level, hence we push the parent to
      // the stack.
      // If the level is not greater than the previous
      // one it means we have popped out n levels of
      // parents, where N is the difference between the 
      // new and the old level. In this case we
      // pop up N parents from the stack.
      if (data.level > previousLevel)
         stack.push_back(data.parent);
      else
         for (size_t pi = 0, pe = previousLevel - data.level; pi != pe; ++pi)
            stack.pop_back();
 
      if (data.matches)
         for (size_t pi = 0, pe = stack.size(); pi != pe; ++pi)
            m_entries[stack[pi]].childMatches = true;

      previousLevel = data.level;
   }
       
   recalculateVisibility();
}

/* the actual structure of the model will not change, only
   its contents, because of the way CMSSW is designed,
   hence this method only needs to be called once.
   */
void FWPSetTableManager::updateSchedule(const edm::ScheduleInfo *info)
{
   if (!m_entries.empty())
      return;
   // Execute only once since the schedule itself
   // cannot be altered.
   assert(m_availablePaths.empty());
   info->availablePaths(m_availablePaths);
         
   for (size_t i = 0, e = m_availablePaths.size(); i != e; ++i)
   {
      PSetData pathEntry;
      const std::string &pathName = m_availablePaths[i];
      pathEntry.label = pathName;
      m_pathIndex.insert(std::make_pair(pathName, m_paths.size()));

      pathEntry.value = "Path";
      pathEntry.level= 0;
      pathEntry.parent = -1;
      pathEntry.path = i;
      pathEntry.editable = false;

      PathInfo pathInfo;
      pathInfo.entryId = m_entries.size();
      pathInfo.passed = false;
      pathInfo.moduleStart = m_modules.size();
      m_paths.push_back(pathInfo);

      m_parentStack.push_back(m_entries.size());
      m_entries.push_back(pathEntry);

      std::vector<std::string> pathModules;
      info->modulesInPath(pathName, pathModules);

      for (size_t mi = 0, me = pathModules.size(); mi != me; ++mi)
      {
         PSetData moduleEntry;

         const edm::ParameterSet* ps = info->parametersForModule(pathModules[mi]);
         const edm::ParameterSet::table& pst = ps->tbl();
         const edm::ParameterSet::table::const_iterator ti = pst.find("@module_edm_type");

         if (ti == pst.end())
            moduleEntry.value = "Unknown module name";
         else
            moduleEntry.value = ti->second.getString();

         moduleEntry.label = pathModules[mi];
         moduleEntry.parent = m_parentStack.back();
         moduleEntry.level = m_parentStack.size();
         moduleEntry.module = mi;
         moduleEntry.path = i;
         moduleEntry.pset = *ps;
         moduleEntry.editable = false;
         ModuleInfo moduleInfo;
         moduleInfo.path = m_paths.size() - 1;
         moduleInfo.entry = m_entries.size();
         moduleInfo.passed = false;
         moduleInfo.dirty = false;
         m_modules.push_back(moduleInfo);
         m_parentStack.push_back(m_entries.size());
         m_entries.push_back(moduleEntry);
         handlePSet(moduleEntry.pset);
         m_parentStack.pop_back();
      }
      m_paths.back().moduleEnd = m_modules.size();
      m_parentStack.pop_back();
   }

   // Nothing is expanded by default.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
      m_entries[i].expanded = false;
   m_filter = "";
}


/** Update the status of a given path. This is the information 
    that changes on event by event basis.
   */
void FWPSetTableManager::update(std::vector<PathUpdate> &pathUpdates)
{
   // Reset all the path / module status information, so that
   // by default paths and modules are considered "not passed".
   for (size_t pi = 0, pe = m_paths.size(); pi != pe; ++pi)
      m_paths[pi].passed = false;
   for (size_t mi = 0, me = m_modules.size(); mi != me; ++mi)
      m_modules[mi].passed = false;
         
   // Update whether or not a given path / module passed selection.
   for (size_t pui = 0, pue = pathUpdates.size(); pui != pue; ++pui)
   {
      PathUpdate &update = pathUpdates[pui];
      std::map<std::string, size_t>::const_iterator index = m_pathIndex.find(update.pathName);
      if (index == m_pathIndex.end())
      {
         std::cerr << "Path " << update.pathName << "cannot be found!" << std::endl;
         continue;
      }
      PathInfo &pathInfo = m_paths[index->second];
      pathInfo.passed = update.passed;
            
      for (size_t mi = pathInfo.moduleStart, me = pathInfo.moduleEnd; mi != me; ++mi)
      {
         ModuleInfo &moduleInfo = m_modules[mi];
         moduleInfo.passed = update.passed || ((mi-pathInfo.moduleStart) < update.choiceMaker);
      }
   }

   implSort(-1, true);
}


int FWPSetTableManager::unsortedRowNumber(int unsorted) const
{
   return unsorted;
}

int FWPSetTableManager::numberOfRows() const {
   return m_row_to_index.size();
}

int FWPSetTableManager::numberOfColumns() const {
   return 2;
}
   
std::vector<std::string> FWPSetTableManager::getTitles() const 
{
   std::vector<std::string> returnValue;
   returnValue.reserve(numberOfColumns());
   returnValue.push_back("Label");
   returnValue.push_back("Value");
   return returnValue;
}
  
FWTableCellRendererBase* FWPSetTableManager::cellRenderer(int iSortedRowNumber, int iCol) const
{
   // If the index is outside the table, we simply return an empty cell.
   // FIXME: how is this actually possible???
   if (static_cast<int>(m_row_to_index.size()) <= iSortedRowNumber)
   {
      m_renderer.setData(std::string(), false);
      return &m_renderer;
   }

   // Do the actual rendering.
   FWTextTreeCellRenderer* renderer;

   int unsortedRow =  m_row_to_index[iSortedRowNumber];
   const PSetData& data = m_entries[unsortedRow];

   std::string value;
   std::string label;

   if (data.level == 0)
   {
      const PathInfo &path = m_paths[data.path];
      label = data.label + " (" + data.value + ")";
       
      value = "";

      if (path.passed)
         renderer = &m_pathPassedRenderer;
      else 
         renderer = &m_pathFailedRenderer;
   }
   else if (data.level == 1)
   {
      const ModuleInfo &module = m_modules[m_paths[data.path].moduleStart + data.module];

      label = data.label + " (" + data.value + ")";
      value = "";

      // "passed" means if module made decision on path 
      // passing or failing
      if (module.passed)
         renderer = &m_modulePassedRenderer;
      else
         renderer = &m_moduleFailedRenderer;
   }
   else
   {
      if (data.type > 0)
         label = data.label + " (" + sTypeTranslations.table_[data.type] + ")";
      else
         label = data.label;
      value = data.value;
         
      if (data.editable)
         renderer = &m_renderer;
      else
         renderer = &m_editingDisabledRenderer;
   }

   // set isParent state for expand icon
   bool isParent = false;
   if (iCol == 0)
   { 
      if (m_filter.empty())
      {
         size_t nextIdx =  unsortedRow + 1;
         isParent = (nextIdx < m_entries.size() &&  m_entries[nextIdx].parent == (size_t)unsortedRow);
      }
      else 
      {
         isParent = data.childMatches;
      }
   } 
   renderer->setIsParent(isParent);

   renderer->setIndentation(0);
   if(data.expanded) {
      renderer->setIsOpen(true);
   } else {
      renderer->setIsOpen(false);        
   }

   if (iCol == 0)
   {
      if (isParent)
         renderer->setIndentation(data.level * 10 );
      else
         renderer->setIndentation(data.level * 10 + 12);

      renderer->setData(label, false);
   }
   else if (iCol == 1)
      renderer->setData(value, false);
   else
      renderer->setData(std::string(), false);

   // If we are rendering the selected cell,
   // we show the editor.
   if (iCol == 1 && iSortedRowNumber == m_selectedRow && iCol == m_selectedColumn)
      renderer->showEditor(data.editable);
   else
      renderer->showEditor(false);

   return renderer;
}


void FWPSetTableManager::setSelection (int row, int column, int mask) {
   if(mask == 4) {
      if( row == m_selectedRow) {
         row = -1;
      }
   }
   changeSelection(row, column);
}

const std::string FWPSetTableManager::title() const {
   return "Modules & their parameters";
}

int FWPSetTableManager::selectedRow() const {
   return m_selectedRow;
}

int FWPSetTableManager::selectedColumn() const {
   return m_selectedColumn;
}
   //virtual void sort (int col, bool reset = false);
bool FWPSetTableManager::rowIsSelected(int row) const 
{
   return m_selectedRow == row;
}


void FWPSetTableManager::recalculateVisibility()
{
   m_row_to_index.clear();

   // Decide about visibility.
   // * If the items are toplevel and they match the filter, they get shown
   //   in any case.
   // * If the item or any of its children match the filter, the item
   //   is visible.
   // * If the filter is empty and the parent is expanded.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
   {
      PSetData &data = m_entries[i];
      if (data.parent == ((size_t) -1))
         data.visible = data.childMatches || data.matches || m_filter.empty();
      else
         data.visible = data.matches || data.childMatches || (m_filter.empty() && m_entries[data.parent].expanded && m_entries[data.parent].visible);

      if (!m_filter.empty())
      {
         //  data.expanded = data.childMatches;
      }
   }

   // Put in the index only the entries which are visible.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
      if (m_entries[i].visible)
         m_row_to_index.push_back(i);
}

void FWPSetTableManager::setExpanded(int row)
{
   if (row == -1)
      return;
   // We do not want to handle expansion
   // events while in filtering mode.
   if (!m_filter.empty())
      return;
   int index = rowToIndex()[row];
   PSetData& data = m_entries[index];

   data.expanded = !data.expanded;
   recalculateVisibility();
   dataChanged();
   visualPropertiesChanged();
}

void FWPSetTableManager::reset() 
{
   changeSelection(-1, -1);
   recalculateVisibility();
   dataChanged();
   visualPropertiesChanged();
}

void FWPSetTableManager::changeSelection(int iRow, int iColumn)
{
   // Nothing changes if we clicked selected
   // twice the same cell.
   if (iRow == m_selectedRow && iColumn == m_selectedColumn)
      return;

   // Otherwise update the selection information
   // and notify observers.
   m_selectedRow = iRow;
   m_selectedColumn = iColumn;

   indexSelected_(iRow, iColumn);
   visualPropertiesChanged();
}

/** Does not apply changes and closes window. */
void FWPSetTableManager::cancelEditor()
{
   if (!m_editor)
      return;
         
   m_editor->UnmapWindow(); 
         
}

 /** This is invoked every single time the
       editor contents must be applied to the selected entry in the pset. 
       @return true on success. 
*/
bool FWPSetTableManager::applyEditor()
{
   if (!m_editor)
      return false;
         
   if (m_selectedRow == -1)
      return false;
   if (m_selectedColumn != 1)
   {
      m_editor->UnmapWindow();
      return false;
   }
         
   PSetData &data = m_entries[m_row_to_index[m_selectedRow]];
   PSetData &parent = m_entries[data.parent];
   try
   {
      switch (data.type)
      {
         case 'I':
            editNumericParameter<int32_t>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'U':
            editNumericParameter<uint32_t>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'D':
            editNumericParameter<double>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'L':
            editNumericParameter<long long>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'X':
            editNumericParameter<unsigned long long>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'S':
            editStringParameter(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'i':
            editVectorParameter<int32_t>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'u':
            editVectorParameter<uint32_t>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'l':
            editVectorParameter<long long>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'x':
            editVectorParameter<unsigned long long>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'd':
            editVectorParameter<double>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 's':
            editVectorParameter<std::string>(parent.pset, data.tracked, data.label, m_editor->GetText());
            break; 
         case 't':
            editInputTag(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'g':
            editESInputTag(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'v':
            editVInputTag(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         case 'F':
            editFileInPath(parent.pset, data.tracked, data.label, m_editor->GetText());
            break;
         default:
            std::cerr << "unsupported parameter" << std::endl;
            m_editor->UnmapWindow();
            return false;
      }
      data.value = m_editor->GetText();
      m_modules[data.module].dirty = true;
      m_editor->UnmapWindow();
   }
   // catch(/*cms::Exception*/ std::exception &e)
   catch(cms::Exception &e)
   {
      m_editor->SetForegroundColor(gVirtualX->GetPixel(kRed));
   }
   return true;
}