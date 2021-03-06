/*=========================================================================

   Program: ParaView
   Module:    pqLightsEditor.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqLightsEditor.h"
#include "ui_pqLightsEditor.h"

#include <QDoubleValidator>
#include "pqApplicationCore.h"
#include "pqNamedWidgets.h"
#include "pqPropertyGroupWidget.h"
#include "pqUndoStack.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSmartPointer.h"

class pqLightsEditor::pqInternal : public Ui::LightsEditor
{
public:
  pqInternal(pqLightsEditor* self)
  {
    this->setupUi (self);
  }
};

namespace {
const char* LIGHT_KIT = "UseLight";
const char* KEY_LIGHT_WARMTH = "KeyLightWarmth";
const char* KEY_LIGHT_INTENSITY = "KeyLightIntensity";
const char* KEY_LIGHT_ELEVATION = "KeyLightElevation";
const char* KEY_LIGHT_AZIMUTH = "KeyLightAzimuth";
const char* FILL_LIGHT_WARMTH = "FillLightWarmth";
const char* FILL_LIGHT_K_F_RATIO = "FillLightK:F Ratio";
const char* FILL_LIGHT_ELEVATION = "FillLightElevation";
const char* FILL_LIGHT_AZIMUTH = "FillLightAzimuth";
const char* BACK_LIGHT_WARMTH = "BackLightWarmth";
const char* BACK_LIGHT_K_B_RATIO = "BackLightK:B Ratio";
const char* BACK_LIGHT_ELEVATION = "BackLightElevation";
const char* BACK_LIGHT_AZIMUTH = "BackLightAzimuth";
const char* HEAD_LIGHT_WARMTH = "HeadLightWarmth";
const char* HEAD_LIGHT_K_H_RATIO = "HeadLightK:H Ratio";
const char* MAINTAIN_LUMINANCE = "MaintainLuminance";

const char* LIGHT = "LightSwitch";
const char* LIGHT_INTENSITY = "LightIntensity";
const char* LIGHT_COLOR = "LightDiffuseColor";

const char* PROPERTY_NAME[] =
{
  LIGHT_KIT,
  KEY_LIGHT_WARMTH,
  KEY_LIGHT_INTENSITY,
  KEY_LIGHT_ELEVATION,
  KEY_LIGHT_AZIMUTH,
  FILL_LIGHT_WARMTH,
  FILL_LIGHT_K_F_RATIO,
  FILL_LIGHT_ELEVATION,
  FILL_LIGHT_AZIMUTH,
  BACK_LIGHT_WARMTH,
  BACK_LIGHT_K_B_RATIO,
  BACK_LIGHT_ELEVATION,
  BACK_LIGHT_AZIMUTH,
  HEAD_LIGHT_WARMTH,
  HEAD_LIGHT_K_H_RATIO,
  MAINTAIN_LUMINANCE,

  LIGHT,
  LIGHT_INTENSITY,
  LIGHT_COLOR
};
const int PROPERTY_COUNT = sizeof(PROPERTY_NAME) / sizeof(PROPERTY_NAME[0]);
}

//-----------------------------------------------------------------------------
pqLightsEditor::pqLightsEditor(
  pqPropertyGroupWidget* propertyWidget,
  QWidget *_parent, Qt::WindowFlags flags):
  Superclass(_parent, flags),
  Internal (new pqInternal (this)),
  PropertyWidget (propertyWidget)
{
  Ui::LightsEditor& ui = *this->Internal;
  pqPropertyGroupWidget& pg = *PropertyWidget;

  pg.addPropertyLink(ui.LightKit,           LIGHT_KIT);
  pg.addPropertyLink(ui.KeyLightWarmth,     KEY_LIGHT_WARMTH);
  pg.addPropertyLink(ui.KeyLightIntensity,  KEY_LIGHT_INTENSITY);
  pg.addPropertyLink(ui.KeyLightElevation,  KEY_LIGHT_ELEVATION);
  pg.addPropertyLink(ui.KeyLightAzimuth,    KEY_LIGHT_AZIMUTH );
  pg.addPropertyLink(ui.FillLightWarmth,    FILL_LIGHT_WARMTH);
  pg.addPropertyLink(ui.FillLightK_F_Ratio, FILL_LIGHT_K_F_RATIO);
  pg.addPropertyLink(ui.FillLightElevation, FILL_LIGHT_ELEVATION);
  pg.addPropertyLink(ui.FillLightAzimuth,   FILL_LIGHT_AZIMUTH);
  pg.addPropertyLink(ui.BackLightWarmth,    BACK_LIGHT_WARMTH );
  pg.addPropertyLink(ui.BackLightK_B_Ratio, BACK_LIGHT_K_B_RATIO);
  pg.addPropertyLink(ui.BackLightElevation, BACK_LIGHT_ELEVATION);
  pg.addPropertyLink(ui.BackLightAzimuth,   BACK_LIGHT_AZIMUTH);
  pg.addPropertyLink(ui.HeadLightWarmth,    HEAD_LIGHT_WARMTH);
  pg.addPropertyLink(ui.HeadLightK_H_Ratio, HEAD_LIGHT_K_H_RATIO);
  pg.addPropertyLink(ui.MaintainLuminance,  MAINTAIN_LUMINANCE);

  pg.addPropertyLink(ui.Light,              LIGHT);
  pg.addPropertyLink(ui.LightIntensity,     LIGHT_INTENSITY);
  pg.addPropertyLink(ui.LightColor,         LIGHT_COLOR);

  connect (ui.Close, SIGNAL(pressed()), this, SLOT(close()));
  connect (ui.Reset, SIGNAL(pressed()), this, SLOT(reset()));
}

//-----------------------------------------------------------------------------
pqLightsEditor::~pqLightsEditor()
{
  delete this->Internal;
  this->Internal = 0;
}

//-----------------------------------------------------------------------------
void pqLightsEditor::reset()
{
  BEGIN_UNDO_SET("Restore Default Lights");
  for (int i = 0; i < PROPERTY_COUNT; ++i)
    {
    vtkSMProperty* _property =
      this->PropertyWidget->propertyGroup()->GetProperty(PROPERTY_NAME[i]);
    _property->ResetToDefault();
    }
  emit this->PropertyWidget->changeFinished();
  END_UNDO_SET();
}
