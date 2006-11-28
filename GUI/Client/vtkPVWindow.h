/*=========================================================================

  Program:   ParaView
  Module:    vtkPVWindow.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVWindow - Main ParaView user interface.
// .SECTION Description
// This class represents a top level window with menu bar and status
// line. It is designed to hold one or more vtkPVViews in it.
// The menu bar contains the following menus:
// @verbatim
// File Menu    -> File manipulations
// View Menu    -> Changes what is displayed on the properties
//                 window (on the left)
// SelectMenu   -> used to select existing data objects
// GlyphMenu    -> used to select existing glyph objects (cascaded from
//                 SelectMenu)
// Help         -> Brings up on-line help
// 
// @endverbatim
// Below the menus, there is a frame separated into two. On the left,
// the "properties" frame is placed and on the right, the render window is
// placed. The "properties" frame is multi-functional. It can display
// general settings, the properties of the current data object/source etc.
// Finally, the status and progress bar (current unused) are packed at
// the bottom.
//
// The PVWindow holds all the existing PVSources (prototypes and instances) 
// and provides access to them. The prototypes are instantiated by
// parsing an XML configuration file. It also has methods to create new data
// objects/sources by cloning prototypes. These data objects can be
// selected either from the SelectMenu or using the navigation window.
// All the reader module prototypes are also stored in the window. The window
// consults each of these when opening a file. The first one which claims
// it can open the current file is passed the filename and is asked to read it.
//
// .SECTION See Also
// vtkPVSource vtkPVData vtkPVRenderView vtkPVNavigationWindow

#ifndef __vtkPVWindow_h
#define __vtkPVWindow_h

#include "vtkKWWindow.h"

#include "vtkClientServerID.h" // Needed for InteractorID

class vtkCollection;
class vtkKWCheckButton;
class vtkKWEntry;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWMenuButton;
class vtkKWPushButton;
class vtkKWPushButtonWithMenu;
class vtkKWRadioButton;
class vtkKWRotateCameraInteractor;
class vtkKWSplitFrame;
class vtkKWToolbar;
class vtkKWToolbarSet;
class vtkPVAnimationInterface;
class vtkPVAnimationManager;
class vtkPVApplication;
class vtkPVApplicationSettingsInterface;
class vtkCameraManipulator;
class vtkPVColorMap;
class vtkPVComparativeVisManagerGUI;
class vtkPVErrorLogDisplay;
class vtkPVGenericRenderWindowInteractor;
class vtkPVInteractorStyle;
class vtkPVInteractorStyleCenterOfRotation;
class vtkPVLookmarkManager;
class vtkPVReaderModule;
class vtkPVRenderView;
class vtkPVSource;
class vtkPVSourceCollection;
class vtkPVTimerLogDisplay;
class vtkPVTraceHelper;
class vtkPVTrackballRoll;
class vtkPVVolumeAppearanceEditor;
class vtkPVWidget;
class vtkPVWindowObserver;
class vtkPVWriter;
class vtkPVXMLPackageParser;
class vtkSMAxesProxy;

//BTX
template <class key, class data> 
class vtkArrayMap;
template <class value>
class vtkLinkedList;
//ETX

#define VTK_PV_SOURCE_MENU_INDEX       2
#define VTK_PV_SOURCE_MENU_LABEL       " Source"
#define VTK_PV_ANIMATION_MENU_INDEX    3
#define VTK_PV_ANIMATION_MENU_LABEL    " Animation"

class VTK_EXPORT vtkPVWindow : public vtkKWWindow
{
public:
  static vtkPVWindow* New();
  vtkTypeRevisionMacro(vtkPVWindow,vtkKWWindow);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Access to the RenderView.
  vtkGetObjectMacro(MainView, vtkPVRenderView);

  // Description:
  // The current source ...  Setting the current source also sets the
  // current PVData.  It also sets the selected composite to the source.
  void SetCurrentPVSource(vtkPVSource *comp);
  void SetCurrentPVSourceCallback(vtkPVSource *comp);
  vtkPVSource *GetCurrentPVSource() {return this->CurrentPVSource;}
  vtkPVSource *GetPreviousPVSource(int idx = 1);
  vtkPVReaderModule *GetCurrentPVReaderModule();

  // Description:
  // This adds a PVSource to the collection called "listname", and makes 
  // it current. No trace entry is added during this call.
  void AddPVSource(const char* listname, vtkPVSource *pvs);

  // Description:
  // This removes a PVSource from the collection called "listname".
  // No trace entry is added during this call.
  void RemovePVSource(const char* listname, vtkPVSource *pvs);

  // Description:
  // This calls delete on all the pv sources.
  void DeleteAllSourcesCallback();
  void DeleteAllSources();
  void DeleteSourceAndOutputs(vtkPVSource*);

  // Description:
  // This removes all the key frames from the animation.
  void DeleteAllKeyframesCallback();
  void DeleteAllKeyframes();
    
  // Description:
  // Find a data source with name "sourcename" in the source list called
  // "listname"
  vtkPVSource *GetPVSource(const char* listname, const char* sourcename);

  // Description:
  // Access methods to ParaView specific menus.
  vtkGetObjectMacro(SelectMenu, vtkKWMenu);
  vtkGetObjectMacro(GlyphMenu, vtkKWMenu);
  vtkGetObjectMacro(SourceMenu, vtkKWMenu);
  vtkGetObjectMacro(FilterMenu, vtkKWMenu);
  
  // Description:
  // Callback for the reset camera button.
  void ResetCameraCallback();
  
  // Description:
  // Callback to show the page for the current source.
  // The callback adds to the trace file, the other does not.
  void ShowCurrentSourceProperties();
  void ShowCurrentSourcePropertiesCallback();

  // Description:
  // Callback to show the animation page.
  void ShowAnimationPanes();

  // Description:
  // Access to the toolbars.
  vtkGetObjectMacro(Toolbar, vtkKWToolbar);
  vtkGetObjectMacro(InteractorToolbar, vtkKWToolbar);
  vtkGetObjectMacro(PickCenterToolbar, vtkKWToolbar);
  vtkGetObjectMacro(LookmarkToolbar, vtkKWToolbar);
 
  // Description:
  // Access from script for regression test.
  void SaveBatchScript(const char *filename);
  void SaveBatchScript(const char *filename, int offScreenFlag, const char* imageFileName, const char* geometryFileName);
  
  // Description:
  // Save the state of the server manager.
  void SaveSMState();
  void SaveSMState(const char *filename);
  
  // Description:
  // Save the pipeline as a tcl script. Callback from menu.
  // Promps for a file name.
  void SaveBatchScript();

  // Description:
  // Like trace, but only final state.
  void SaveState(const char *filename);

  // Description:
  // Save the state as a tcl script. Callback from menu.
  // Promps for a file name.
  void SaveState();

  // Description:
  // Copy the trace file to a file with the specified file name.
  void SaveTrace();
  int SaveTrace(const char*);
  
  // Description:
  // Open a data file. Prompt the user for the filename first.
  void OpenCallback();

  // Description:
  // Open a data file. Does not prompt the user. Returns VTK_OK on
  // success and VTK_ERROR on failure. Set store to 1 to store to the
  // recently used files list. The reader modules are given the
  // filename and asked if they can read the file, the first module
  // capable of reading is given the task.
  int Open(char *fileName, int store);
  int Open(char *fileName) { return this->Open(fileName, 0); }

  // Description:
  // Open a data file give a filename and a reader module. Unlike
  // Open(), OpenCustom() does not use CanReadFile() to determine
  // which module can open the file. Instead, the first module which
  // matches the name "reader" gets to open the file.
  int OpenCustom(const char* reader, const char* filename);

  // Description:
  // Used mainly by the scripting interface, these three methods are
  // normally called in order during the file opening process. Given
  // the reader module name, InitializeReadCustom() returns a clone
  // which can be passed to ReadFileInformation() and FinalizeRead()
  // to finish the reading process.
  vtkPVReaderModule* InitializeReadCustom(const char* reader, 
                                          const char* fileName);
  int FinalizeRead       (vtkPVReaderModule* clone, const char *fileName);
  int ReadFileInformation(vtkPVReaderModule* clone, const char *fileName);

  // Description:
  // Play the demo.
  void PlayDemo();

  // Description:
  // Stuff for creating a log file for times.
  void ShowTimerLog();
  void ShowErrorLog();

  // Description:
  // Show the comparative vis gui.
  void ShowComparativeVisManager();
 
  // Description:
  // Callback for saving animation as images.
  void SaveAnimation();

  // Description:
  // Callback for saving animation geometry in a series of .vtp files.
  void SaveGeometry();

  // Description:
  // Callback fronm the file menus "SaveData" entry.
  // It pops up a dialog to get the filename.
  void WriteData();
  
  // Description:
  // Methods that can be used from scripts to save data to files.
  void WriteVTKFile(const char* filename, int ghostLevel, int timeSeries);
  void WriteVTKFile(const char* filename, int ghostLevel)
    { this->WriteVTKFile(filename, ghostLevel, 0); }
  void WriteVTKFile(const char* filename)
    { this->WriteVTKFile(filename, 0); }

  // Description:
  // These methods create a new data source/object given a name and a 
  // source list.
  vtkPVSource *CreatePVSource(const char *className)
    { return this->CreatePVSource(className, 0, 1, 1); }
  vtkPVSource *CreatePVSource(const char *className, const char* sourceList)
    { return this->CreatePVSource(className, sourceList, 1, 1); }
  vtkPVSource *CreatePVSource(const char *className, const char* sourceList,
                              int add_trace_entry, int grabFocus);
  
  //BTX
  // Description:
  // Access to the interactor styles from tcl.
  vtkGetObjectMacro(CameraStyle3D, vtkPVInteractorStyle);
  vtkGetObjectMacro(CameraStyle2D, vtkPVInteractorStyle);
  //ETX

  // Description:
  // Get the source list called "listname". The default source
  // lists are "Sources" and "GlyphSources".
  vtkPVSourceCollection *GetSourceList(const char* listname);

  // Description: 
  // Re-populate the select menu from the list of existing data
  // objects.
  void UpdateSelectMenu();

  // Description:
  // Ability to disable and enable the filter buttons on the toolbar.
  // Most of the manipulation is internal to window.
  void DisableToolbarButtons();
  void EnableToolbarButtons();
  void EnableToolbarButton(const char* buttonName);
  void DisableToolbarButton(const char* buttonName);

  // Description:
  // Re-populate the source menu.
  void UpdateSourceMenu();

  // Description:
  // Re-populate the filter menu.
  void UpdateFilterMenu();
  
  // Description:
  // Access to the animation interface for scripting.
  vtkGetObjectMacro(AnimationManager, vtkPVAnimationManager);

  // Description:
  // Add a prototype from which a module can be created.
  void AddPrototype(const char* name, vtkPVSource* prototype);

  // Description:
  // Add a push button to the main toolbar.
  void AddToolbarButton(const char* buttonName, const char* imageName,
                        const char* fileName, const char* command, 
                        const char* balloonHelp, int buttonVisibility);

  // Description:
  // Determines whether message dialogs or error macros as used
  // for displaying certain warnings/errors.
  vtkSetMacro(UseMessageDialog, int);
  vtkGetMacro(UseMessageDialog, int);
  vtkBooleanMacro(UseMessageDialog, int);

  // Description:
  // Set for running the demo to make the gui a little faster.
  vtkSetMacro(InDemo, int);
  vtkGetMacro(InDemo, int);

  // Description:
  // Set to true when in comparative vis mode
  vtkSetMacro(InComparativeVis, int);
  vtkGetMacro(InComparativeVis, int);

  // Description:
  // Adds manipulator to the list of available manipulators.
  void AddManipulator(const char* type, const char* name, 
                      vtkCameraManipulator*);
  
  // Description:
  // Adds manipulators variable and the widget to the list.
  void AddManipulatorArgument(const char* type, const char* name, 
                              const char* variable, vtkPVWidget* widget);

  // Description:
  // Whether to parse and create the default interfaces at startup
  vtkSetMacro(InitializeDefaultInterfaces, int);
  vtkGetMacro(InitializeDefaultInterfaces, int);
  vtkBooleanMacro(InitializeDefaultInterfaces, int);

  // Description:
  // Open a ParaView package (prompt the user for the filename) 
  // and load the contents. Returns VTK_OK on success.
  int OpenPackage();
  int OpenPackage(const char* fileName);

  // Description:
  // Open an image file into ParaView (prompt the user for the filename)
  // and load it as a texture. Returns VTK_OK on success.
  int LoadTexture();
  int LoadTexture(const char* fileName);

  // Description:
  // Load in a Tcl based script to drive the application.
  virtual void LoadScript() { this->Superclass::LoadScript(); }
  virtual void LoadScript(const char *name);

  // Description:
  // Callbacks for generic render window interactor
  void MouseAction(int action,int button, int x,int y, int shift,int control);
  void Configure(int width, int height);
  void KeyAction(char keyCode, int x, int y);

  // Description:
  // Set/Get the current interactor style. 
  void SetInteractorStyle(int iStyle);
  int GetInteractorStyle();

  // Description
  // Call SetInteractorStyle(). Legacy method.
  void ChangeInteractorStyle(int index);

  // Description:
  // Callbacks for center of rotation widgets
  void CenterEntryOpenCallback();
  void CenterEntryCloseCallback();
  void CenterEntryCallback();
  void ResetCenterCallback();
  void ToggleCenterActorCallback();

  // Description
  // Access to these widgets from outside vtkPVWindow
  // (in vtkPVInteractorStyleCenterOfRotation)
  vtkGetObjectMacro(CenterXEntry, vtkKWEntry);
  vtkGetObjectMacro(CenterYEntry, vtkKWEntry);
  vtkGetObjectMacro(CenterZEntry, vtkKWEntry);

  // Description:
  // Popup the vtk warning message
  virtual void WarningMessage(const char* message);
  
  // Description:
  // Popup the vtk error message
  virtual void ErrorMessage(const char* message);

  // Description:
  // This method returns a color map for a specific global parameter
  // (e.g. Temperature).
  vtkPVColorMap* GetPVColorMap(const char* parameterName, 
                               int numberOfComponents);

  // Description:
  // This method returns the shared volume appearance editor
  vtkPVVolumeAppearanceEditor *GetVolumeAppearanceEditor();
  
  // Description:
  // Return the collection of all colormaps.
  vtkCollection* GetPVColorMaps();

  // Description:
  // Propagates the center to the manipulators.
  void SetCenterOfRotation(float f[3]) 
    { this->SetCenterOfRotation(f[0], f[1], f[2]); }
  void SetCenterOfRotation(float x, float y, float z);

  //BTX
  // Description:
  // Access to this style from outside vtkPVWindow
  vtkGetObjectMacro(CenterOfRotationStyle, vtkPVInteractorStyleCenterOfRotation);
  //ETX
  
  // Description:
  // These methods enable and disable the navigation window and
  // selection window.
  void EnableNavigationWindow();
  void DisableNavigationWindow();

  // Description:
  // Called by PV3DWidgets.
  // All inteteractor stuff should be moved to 
  // PVRenderView or PVRenderModule !!!!!!!!
  virtual void SetInteractiveRenderEnabled(int s);
  vtkBooleanMacro(InteractiveRenderEnabled,int);
  vtkGetMacro(InteractiveRenderEnabled,int);

  // Description:
  // Read interface description from XML.
  void ReadSourceInterfacesFromString(const char*);
  
  // Description:
  // Accessor for the linked list. This is not available from Tcl,
  // since vtkLinkedList is templated.
  //BTX
  vtkLinkedList<vtkPVReaderModule*>* GetReaderList() { return this->ReaderList; }
  //ETX

  // Description:
  // Get/Show the Application Settings Interface. 
  virtual vtkKWApplicationSettingsInterface *GetApplicationSettingsInterface();

  //BTX
  // Description:
  // I am using this method instead of MakeTclObject.
  void SetInteractor(vtkPVGenericRenderWindowInteractor *iren);
  vtkPVGenericRenderWindowInteractor* GetInteractor() {return this->Interactor;}
  //ETX

  vtkClientServerID GetInteractorID(){ return this->InteractorID;}

  // Description:
  // Returns 1 is file exists on the server and is readable and 0
  // otherwise.
  int CheckIfFileIsReadable(const char* fname);

  // Description:
  // Access to these widgets so their state can be set from a script
  vtkGetObjectMacro(TranslateCameraButton, vtkKWRadioButton);
  vtkGetObjectMacro(RotateCameraButton, vtkKWRadioButton);
  vtkGetObjectMacro(ResetCameraButton, vtkKWPushButtonWithMenu);
  
  // Description:
  // Update the toolbar aspect once the toolbar settings have been changed
  virtual void UpdateToolbarState();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();
  virtual void UpdateMenuState();
  virtual int GetEnabled();

  // Description:
  // Set the progress. All progresses 0 or 100 will be ignored.
  void SetProgress(const char* text, int val);
  void StartProgress();
  void EndProgress();

  // Description:
  // The following methods are needed for mouse interaction tracing.
  void SetInteractorEventInformation(int x, int y, int ctrl, int shift,
                                     char keycode, int repeatcount,
                                     const char *keysym);
  void InvokeInteractorEvent(const char *event);

  // Description:
  // Call AcceptCallback() on the current source.
  void AcceptCurrentSource();

  // Description:
  // Callback for when the user changed the visibility of a toolbar button.
  void ToolbarMenuCheckCallback(const char* buttonName);

  // Description:
  // Display the Lookmark Manager.
  void DisplayLookmarkManager();

  vtkGetObjectMacro(PVLookmarkManager, vtkPVLookmarkManager);

  // Description: 
  // When on and SaveState() is called, only sources that "contribute to the view" are saved to the session script.
  vtkSetMacro(SaveVisibleSourcesOnlyFlag,int);

  // Description:
  // Method to change toolbar state in tcl scripts.
  // identifier: "animation" : animation toolbar.
  //             "interaction" : interaction toolbar
  //             "tools"       : tools toolbar
  //             "camera"      : camera toolbar.
  void SetToolbarVisibility(const char* identifier, int state);

  // Description:
  // This changes the visibility of the Horizontal pane.
  // Animation key frames are shown in the Horizontal pane.
  virtual void SetSecondaryPanelVisibility(int);

  // Description:
  // Called to add a default animation for a source.
  // vtkPVSource calls this when the source is initialized.
  void AddDefaultAnimation(vtkPVSource* pvSource);

  // Description:
  // Calls an Update on the Animation Manager forcing it to get in sync with the 
  // current system state. Called when a source is created or deleted.
  void UpdateAnimationInterface();

  // Description:
  // Get the trace helper framework.
  vtkGetObjectMacro(TraceHelper, vtkPVTraceHelper);

  // Description:
  // This method is called when error icon is clicked.
  void ErrorIconCallback();
    
  // Description:
  // Returns the comparative vis. manager interface.
  vtkPVComparativeVisManagerGUI* GetComparativeVisManagerGUI();
  
//BTX
  enum InteractorStyles
  {
    INTERACTOR_STYLE_UNKNOWN = 0,
    INTERACTOR_STYLE_3D = 1,
    INTERACTOR_STYLE_2D = 2,
    INTERACTOR_STYLE_TRACKBALL = 3,
    INTERACTOR_STYLE_CENTER_OF_ROTATION = 4
  };
//ETX

  // Description:
  // Show a main user interface given its name. The name is the name returned
  // by the GetName() method of a vtkKWUserInterfacePanel (UIP). The 
  // main UserInterfaceManager (UIM) will be queried to check if it is indeed
  // managing an UIP with that name, and show/raise that UIP accordingly.
  virtual void ShowMainUserInterface(const char *name)
    { this->Superclass::ShowMainUserInterface(name); }

  // Description:
  // Resets all the settings to default values.
  void ResetSettingsToDefault();

  // Description:
  // Get the Id for the most recent state saved.
  vtkGetMacro(StateFileId, vtkIdType);

protected:
  vtkPVWindow();
  ~vtkPVWindow();

  // Description:
  // Create the window and all of the associated widgets. This
  // essentially creates the whole user interface. ParaView supports
  // only one window.
  virtual void CreateWidget();

  static const char* ComparativeVisMenuLabel;

  int OpenWithReader(const char *fileName, vtkPVReaderModule* reader);
  vtkPVReaderModule* InitializeRead(vtkPVReaderModule* proto, 
                                    const char *fileName);
  // Description:
  // Save/Restore window geometry
  virtual void SaveWindowGeometryToRegistry();

  // Description:
  // Restore the PVWindow geometry alone.
  virtual void RestorePVWindowGeometry();

  // Main render window
  vtkPVRenderView *MainView;

  // Should I move this to vtkPVRenderView?
  vtkPVGenericRenderWindowInteractor *Interactor;
  //InteractorID is a duplicate of InteractorID in PVRenderModule.
  //Need to move the logic for InteractorID to PVRenderModule
  vtkClientServerID InteractorID;
  
  // ParaView specific menus
  vtkKWMenu *SourceMenu;
  vtkKWMenu *FilterMenu;
  vtkKWMenu *SelectMenu;
  vtkKWMenu *GlyphMenu;

  vtkPVInteractorStyle *CameraStyle2D;
  vtkPVInteractorStyle *CameraStyle3D;

  // This should be made into a 3D Widget.
  vtkPVInteractorStyleCenterOfRotation *CenterOfRotationStyle;

  // Interactor stuff
  vtkKWToolbar *InteractorToolbar;
  vtkKWPushButtonWithMenu *ResetCameraButton;
  vtkKWRadioButton *RotateCameraButton;
  vtkKWRadioButton *TranslateCameraButton;
    
  // Main toolbar
  vtkKWToolbar* Toolbar;
  // This controls button visibility.
  vtkKWMenuButton* ToolbarMenuButton;
 
  // widgets for setting center of rotation for rotate camera interactor style
  vtkKWToolbar *PickCenterToolbar;
  vtkKWPushButton *PickCenterButton;
  vtkKWPushButton *ResetCenterButton;
  vtkKWPushButton *HideCenterButton;
  vtkKWPushButton *CenterEntryOpenCloseButton;
  vtkKWFrame *CenterEntryFrame;
  vtkKWLabel *CenterXLabel;
  vtkKWEntry *CenterXEntry;
  vtkKWLabel *CenterYLabel;
  vtkKWEntry *CenterYEntry;
  vtkKWLabel *CenterZLabel;
  vtkKWEntry *CenterZEntry;
    
  void ResizeCenterActor();
  
  // Used internally.  Down casts vtkKWApplication to vtkPVApplication
  vtkPVApplication *GetPVApplication();

  // Separating out creation of the main view.
  void CreateMainView(vtkPVApplication *pvApp);
  
  // Description:
  // Deallocate/delete/reparent some internal objects in order to solve
  // reference loops that would prevent this instance from being deleted.
  virtual void PrepareForDelete();

  vtkPVSource *CurrentPVSource;

  // The animation interface. I put it in window because
  // if we ever get more that one renderer, the animation
  // will save out the window with all renderers.
  vtkPVAnimationManager* AnimationManager;

  // Initialization methods called from create.
  virtual void InitializeMenus();
  void InitializeInteractorInterfaces();

  vtkPVTimerLogDisplay *TimerLogDisplay;
  vtkPVErrorLogDisplay *ErrorLogDisplay;

  // Description:
  // Create the comparative vis. manager interface. This is normally called
  // by ShowComparativeVisManager() but can be called separately if a
  // manager has to be created without showing it.
  void CreateComparativeVisManagerGUI();

  // Extensions of files that loaded readers recognize.
  char *FileExtensions;
  char *FileDescriptions;
  // Add a file type and the corresponding prototype
  void AddFileType(const char* description, const char* ext, 
                   vtkPVReaderModule* prototype);

  // Read interface description from XML.
  void ReadSourceInterfaces();
  void ReadSourceInterfacesFromFile(const char*);
  int ReadSourceInterfacesFromDirectory(const char*);

  // Add a file writer.
  void AddFileWriter(vtkPVWriter* writer);  
  
  // Helper for WriteVTKFile() and WritePVTKFile().
  vtkPVWriter* FindPVWriter(const char* fileName, int parallel, int numParts); 

//BTX
  vtkArrayMap<const char*, vtkPVSource*>* Prototypes;
  vtkArrayMap<const char*, vtkPVSourceCollection*>* SourceLists;
  vtkArrayMap<const char*, vtkKWPushButton*>* ToolbarButtons;
  vtkArrayMap<const char*, const char*>* Writers;
  vtkArrayMap<const char*, int>* MenuState;
  vtkLinkedList<vtkPVReaderModule*>* ReaderList;
  vtkLinkedList<const char*>* PackageFiles;
  vtkLinkedList<vtkPVWriter*>* FileWriterList;

  friend class vtkPVXMLPackageParser;
//ETX

  vtkCollection *PVColorMaps;

  vtkPVVolumeAppearanceEditor *VolumeAppearanceEditor;
  
  // This can be used to disable the pop-up dialogs if necessary
  // (usually used from inside regression scripts)
  int UseMessageDialog;

  // Whether or not to read the default interfaces.
  int InitializeDefaultInterfaces;

  // Description:
  // Utility function which return the position of the first '.'
  // from the right. Note that this returns a pointer offset
  // from the original pointer. DO NOT DELETE THE ORIGINAL POINTER
  // while using the extension.
  const char* ExtractFileExtension(const char* fname);

  // Description:
  // Create error log display.
  void CreateErrorLogDisplay();

  void HideCenterActor();
  void ShowCenterActor();

  int CenterActorVisibility;

  int InteractiveRenderEnabled;

  // Description:
  // Update the image in the status frame. Usually a logo of some sort.
  virtual void UpdateStatusImage();

  // To avoid disabling menus multiple times.
  int MenusDisabled;
  int ToolbarButtonsDisabled;

  int InDemo;
  int InComparativeVis;

  double LastProgress;
  int ExpectProgress;
  int ModifiedEnableState;

  vtkClientServerID ServerFileListingID;

  vtkSMAxesProxy *CenterAxesProxy; 
  char* CenterAxesProxyName;
  vtkSetStringMacro(CenterAxesProxyName);

  vtkPVTraceHelper* TraceHelper;

  int SaveVisibleSourcesOnlyFlag;

  // When ever a PV GUI state file is being saved, we assign it a unique Id. 
  // This Id helps the vtkPVTraceHelper to realize if the initialization
  // code should be added to the state file (or if it has been already added.)
  vtkIdType StateFileId;

  vtkKWToolbar *LookmarkToolbar;
  vtkKWPushButton *LookmarkButton;
  vtkPVLookmarkManager *PVLookmarkManager;

  // Description:
  // Show a main user interface. The  main UserInterfaceManager (UIM) will be
  // queried to check if it is indeedmanaging the UIP, and show/raise that UIP
  // accordingly.
  virtual void ShowMainUserInterface(vtkKWUserInterfacePanel *panel);

private:
  vtkPVComparativeVisManagerGUI* ComparativeVisManagerGUI;
  vtkPVWindowObserver* Observer;

  vtkPVWindow(const vtkPVWindow&); // Not implemented
  void operator=(const vtkPVWindow&); // Not implemented
};


#endif
