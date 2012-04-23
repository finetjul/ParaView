/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRepresentedDataStorageInternals
// .SECTION Description

#ifndef __vtkRepresentedDataStorageInternals_h
#define __vtkRepresentedDataStorageInternals_h

#include "vtkCompositeRepresentation.h"
#include "vtkDataObject.h"
#include "vtkPVDataRepresentation.h"
#include "vtkPVTrivialProducer.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"

#include <map>
#include <utility>

class vtkRepresentedDataStorage::vtkInternals
{
public:
  class vtkItem
    {
    vtkSmartPointer<vtkPVTrivialProducer> Producer;
    vtkWeakPointer<vtkDataObject> DataObject;
    unsigned long TimeStamp;
    unsigned long ActualMemorySize;
  public:
    vtkWeakPointer<vtkPVDataRepresentation> Representation;
    bool AlwaysClone;
    bool Redistributable;

    vtkItem() :
      Producer(vtkSmartPointer<vtkPVTrivialProducer>::New()),
      TimeStamp(0),
      ActualMemorySize(0),
      AlwaysClone(false),
      Redistributable(false)
    { }

    void SetDataObject(vtkDataObject* data)
      {
      this->DataObject = data;
      this->Producer->SetOutput(data);
      // the vtkTrivialProducer's MTime is is changed whenever the data itself
      // changes or the data's mtime changes and hence we get a good indication
      // of when we have a new piece for real.
      this->TimeStamp = this->Producer->GetMTime();
      this->ActualMemorySize = data? data->GetActualMemorySize() : 0;
      }

    vtkPVTrivialProducer* GetProducer() const
      { return this->Producer.GetPointer(); }
    vtkDataObject* GetDataObject() const
      { return this->DataObject.GetPointer(); }
    unsigned long GetTimeStamp() const
      { return this->TimeStamp; }
    unsigned long GetVisibleDataSize()
      {
      if (this->Representation && this->Representation->GetVisibility())
        {
        return this->ActualMemorySize;
        }
      return 0;
      }
    };

  typedef std::map<int, std::pair<vtkItem, vtkItem> > ItemsMapType;

  vtkItem* GetItem(int index, bool use_second)
    {
    if (this->ItemsMap.find(index) != this->ItemsMap.end())
      {
      return use_second? &(this->ItemsMap[index].second) :
        &(this->ItemsMap[index].first);
      }
    return NULL;
    }

  vtkItem* GetItem(vtkPVDataRepresentation* repr, bool use_second)
    {
    RepresentationToIdMapType::iterator iter =
      this->RepresentationToIdMap.find(repr);
    if (iter != this->RepresentationToIdMap.end())
      {
      int index = iter->second;
      return use_second? &(this->ItemsMap[index].second) :
        &(this->ItemsMap[index].first);
      }

    // We have to this wackiness for vtkCompositeRepresentation.
    // vtkCompositeRepresentation doesn't add it's "active" representation to
    // the view, but instead forwards call to it. That implies that the item
    // registered with the vtkRepresentedDataStorage is not the same as the item
    // used when passing data using SetPiece() and such calls. Hence we handle
    // vtkCompositeRepresentation specially.
    ItemsMapType::iterator iter2;
    for (iter2 = this->ItemsMap.begin(); iter2 != this->ItemsMap.end(); ++iter2)
      {
      vtkItem& item = use_second? iter2->second.second : iter2->second.first;

      vtkCompositeRepresentation* composite = vtkCompositeRepresentation::SafeDownCast(
        item.Representation.GetPointer());
      if (composite && composite->GetActiveRepresentation() == repr)
        {
        return &item;
        }
      }
    return NULL;
    }

  unsigned long GetVisibleDataSize(bool use_second_if_available)
    {
    unsigned long size = 0;
    ItemsMapType::iterator iter;
    for (iter = this->ItemsMap.begin(); iter != this->ItemsMap.end(); ++iter)
      {
      if (use_second_if_available && iter->second.second.GetDataObject())
        {
        size += iter->second.second.GetVisibleDataSize();
        }
      else
        {
        size += iter->second.first.GetVisibleDataSize();
        }
      }
    return size;
    }

  typedef std::map<vtkPVDataRepresentation*, int>
    RepresentationToIdMapType;

  RepresentationToIdMapType RepresentationToIdMap;
  ItemsMapType ItemsMap;
};
#endif
