function varargout = coolLED(varargin)
% COOLLED M-file for coolLED.fig
%      COOLLED, by itself, creates a new COOLLED or raises the existing
%      singleton*.
%
%      H = COOLLED returns the handle to a new COOLLED or the handle to
%      the existing singleton*.
%
%      COOLLED('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in COOLLED.M with the given input arguments.
%
%      COOLLED('Property','Value',...) creates a new COOLLED or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before coolLED_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to coolLED_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help coolLED

% Last Modified by GUIDE v2.5 05-Oct-2011 19:26:38

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @coolLED_OpeningFcn, ...
                   'gui_OutputFcn',  @coolLED_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT

% ------------------------------------------------------------------
% --- Executes just before coolLED is made visible.
function coolLED_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to coolLED (see VARARGIN)

% Choose default command line output for coolLED
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes coolLED wait for user response (see UIRESUME)
% uiwait(handles.figure1);
return;

% ------------------------------------------------------------------
% --- Outputs from this function are returned to the command line.
function varargout = coolLED_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;
return;

% ------------------------------------------------------------------
function led4Intensity_Callback(hObject, eventdata, handles)

[intensities, led4Intensity, pos] = getLocalBatch(progmanager, hObject, 'intensities', 'led4Intensity', 'positionNumber');
intensities(pos, 4) = led4Intensity;
setLocal(progmanager, hObject, 'intensities', intensities);

return;

% ------------------------------------------------------------------
% --- Executes during object creation, after setting all properties.
function led4Intensity_CreateFcn(hObject, eventdata, handles)
% hObject    handle to led4Intensity (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
return;

% ------------------------------------------------------------------
% --- Executes on button press in led4Armed.
function led4Armed_Callback(hObject, eventdata, handles)

[armedArray, led4Armed, pos] = getLocalBatch(progmanager, hObject, 'armedArray', 'led4Armed', 'positionNumber');
armedArray(pos, 4) = led4Armed;
setLocal(progmanager, hObject, 'armedArray', armedArray);

return;

% ------------------------------------------------------------------
function led3Intensity_Callback(hObject, eventdata, handles)

[intensities, led3Intensity, pos] = getLocalBatch(progmanager, hObject, 'intensities', 'led3Intensity', 'positionNumber');
intensities(pos, 3) = led3Intensity;
setLocal(progmanager, hObject, 'intensities', intensities);

return;

% ------------------------------------------------------------------
% --- Executes during object creation, after setting all properties.
function led3Intensity_CreateFcn(hObject, eventdata, handles)
% hObject    handle to led3Intensity (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
return;

% ------------------------------------------------------------------
% --- Executes on button press in led3Armed.
function led3Armed_Callback(hObject, eventdata, handles)

[armedArray, led3Armed, pos] = getLocalBatch(progmanager, hObject, 'armedArray', 'led3Armed', 'positionNumber');
armedArray(pos, 3) = led3Armed;
setLocal(progmanager, hObject, 'armedArray', armedArray);

return;

% ------------------------------------------------------------------
function led2Intensity_Callback(hObject, eventdata, handles)

[intensities, led2Intensity, pos] = getLocalBatch(progmanager, hObject, 'intensities', 'led2Intensity', 'positionNumber');
intensities(pos, 2) = led2Intensity;
setLocal(progmanager, hObject, 'intensities', intensities);

return;

% ------------------------------------------------------------------
% --- Executes during object creation, after setting all properties.
function led2Intensity_CreateFcn(hObject, eventdata, handles)
% hObject    handle to led2Intensity (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
return;

% ------------------------------------------------------------------
% --- Executes on button press in led2Armed.
function led2Armed_Callback(hObject, eventdata, handles)

[armedArray, led2Armed, pos] = getLocalBatch(progmanager, hObject, 'armedArray', 'led2Armed', 'positionNumber');
armedArray(pos, 2) = led2Armed;
setLocal(progmanager, hObject, 'armedArray', armedArray);

return;

% ------------------------------------------------------------------
% --- Executes on slider movement.
function positionUpSlider_Callback(hObject, eventdata, handles)

pos = getLocal(progmanager, hObject, 'positionNumber');
setLocalBatch(progmanager, hObject, 'positionUpSlider', 0, 'positionNumber', pos + 1);
position_Callback(hObject, eventdata, handles);

return;

% ------------------------------------------------------------------
% --- Executes during object creation, after setting all properties.
function positionUpSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to positionUpSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end
return;

% ------------------------------------------------------------------
% --- Executes on slider movement.
function positionDownSlider_Callback(hObject, eventdata, handles)

pos = getLocal(progmanager, hObject, 'positionNumber');
setLocalBatch(progmanager, hObject, 'positionDownSlider', 1, 'positionNumber', pos - 1);
position_Callback(hObject, eventdata, handles);

return;

% ------------------------------------------------------------------
% --- Executes during object creation, after setting all properties.
function positionDownSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to positionDownSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end
return;

% ------------------------------------------------------------------
function position_Callback(hObject, eventdata, handles)

[position, intensities, armedArray] = getLocalBatch(progmanager, hObject, 'positionNumber', 'intensities', 'armedArray');

if position < 1
    position = 1;
    setLocal(progmanager, hObject, 'positionNumber', position);
elseif position > size(intensities, 1)
    position = size(intensities, 1);
    setLocal(progmanager, hObject, 'positionNumber', position);
end

if size(intensities, 1) >= position && position > 0
    setLocalBatch(progmanager, hObject, 'led1Intensity', intensities(position, 1), 'led2Intensity', intensities(position, 2), ...
        'led3Intensity', intensities(position, 3), 'led4Intensity', intensities(position, 4));
end

if size(armedArray, 1) >= position && position > 0
    setLocalBatch(progmanager, hObject, 'led1Armed', armedArray(position, 1), 'led2Armed', armedArray(position, 2), ...
        'led3Armed', armedArray(position, 3), 'led4Armed', armedArray(position, 4));
end

return;

% ------------------------------------------------------------------
% --- Executes during object creation, after setting all properties.
function position_CreateFcn(hObject, eventdata, handles)
% hObject    handle to position (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
return;

% ------------------------------------------------------------------
function led1Intensity_Callback(hObject, eventdata, handles)

[intensities, led1Intensity, pos] = getLocalBatch(progmanager, hObject, 'intensities', 'led1Intensity', 'positionNumber');
intensities(pos, 1) = led1Intensity;
setLocal(progmanager, hObject, 'intensities', intensities);

return;

% ------------------------------------------------------------------
% --- Executes during object creation, after setting all properties.
function led1Intensity_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
return;

% ------------------------------------------------------------------
% --- Executes on button press in led1Armed.
function led1Armed_Callback(hObject, eventdata, handles)

[armedArray, led1Armed, pos] = getLocalBatch(progmanager, hObject, 'armedArray', 'led1Armed', 'positionNumber');
armedArray(pos, 1) = led1Armed;
setLocal(progmanager, hObject, 'armedArray', armedArray);

return;

% ------------------------------------------------------------------
% --- Executes on button press in addPosition.
function addPosition_Callback(hObject, eventdata, handles)

[intensities, armedArray, pos] = getLocalBatch(progmanager, hObject, 'intensities', 'armedArray', 'positionNumber');
intensities(size(intensities, 1) + 1, 1:4) = 0;
armedArray(size(armedArray, 1) + 1, 1:4) = 0;
pos = size(intensities, 1);
setLocalBatch(progmanager, hObject, 'intensities', intensities, 'armedArray', armedArray, 'positionNumber', pos);

genericUpdateFcn(hObject, eventdata, handles);
position_Callback(hObject, eventdata, handles);

return;

% ------------------------------------------------------------------
% --- Executes on button press in deletePosition.
function deletePosition_Callback(hObject, eventdata, handles)
% eventData.eventType='loopIterate'; loopListener(hObject, eventData); return;
[intensities, armedArray, pos] = getLocalBatch(progmanager, hObject, 'intensities', 'armedArray', 'positionNumber');

intensities = intensities([1:pos-1, pos+1:size(intensities, 1)], 1:4);
armedArray = armedArray([1:pos-1, pos+1:size(armedArray, 1)], 1:4);

setLocalBatch(progmanager, hObject, 'intensities', intensities, 'armedArray', armedArray, 'positionNumber', pos);

genericUpdateFcn(hObject, eventdata, handles);
position_Callback(hObject, eventdata, handles);

return;

% ------------------------------------------------------------------
function out = makeGlobalCellArray(hObject, eventdata, handles)

out = {
       'hObject', hObject, ...
       'led1Intensity', 0, 'Class', 'numeric', 'Gui', 'led1Intensity', ...
       'led1Armed', 0, 'Class', 'numeric', 'Gui', 'led1Armed', ...
       'led2Intensity', 0, 'Class', 'numeric', 'Gui', 'led2Intensity', ...
       'led2Armed', 0, 'Class', 'numeric', 'Gui', 'led2Armed', ...
       'led3Intensity', 0, 'Class', 'numeric', 'Gui', 'led3Intensity', ...
       'led3Armed', 0, 'Class', 'numeric', 'Gui', 'led3Armed', ...
       'led4Intensity', 0, 'Class', 'numeric', 'Gui', 'led4Intensity', ...
       'led4Armed', 0, 'Class', 'numeric', 'Gui', 'led4Armed', ...
       'positionNumber', 0, 'Class', 'numeric', 'Gui', 'position', 'Config', 7, ...
       'positionUpSlider', 0, 'Class', 'numeric', 'Gui', 'positionUpSlider', ...
       'positionDownSlider', 1, 'Class', 'numeric', 'Gui', 'positionDownSlider', ...
       'intensities', [], 'Class', 'numeric', 'Config', 7, ...
       'armedArray', [], 'Class', 'numeric', 'Config', 7, ...
       'enable', 0, 'Class', 'numeric', 'Gui', 'enable', 'Config', 7, ...
       'ledObj', [], ...
   };

return;

% ------------------------------------------------------------------
function genericStartFcn(hObject, eventdata, handles)

[lg, lm] = lg_factory;
registerLoopable(lm, {@loopListener, hObject}, 'coolLEDGui');
setLocal(progmanager, hObject, 'ledObj', Dabs.Devices.CoolLED.PE(3));

return;

% ------------------------------------------------------------------
function loopListener(hObject, loopEvent)

[enable] = getLocalBatch(progmanager, hObject, 'enable');

if enable
    switch loopEvent.eventType
        case 'loopStart'
        case 'loopStartPreciseTiming'
        case 'loopIteration'
            updateCoolLED(hObject);
            incrementPosition(hObject);
        case 'loopStop'
        otherwise
            fprintf(2, 'Warning - coolLED encountered an unrecognized loop event type: %s\n', loopEvent.eventType);
    end
end

return;

% ------------------------------------------------------------------
function updateCoolLED(hObject)

[intensities, armedArray, pos, pe, enable] = getLocalBatch(progmanager, hObject, 'intensities', 'armedArray', 'positionNumber', 'ledObj', 'enable');
% msgbox(sprintf('Update CoolLED...\nIntensities: %s\nArmed: %s', mat2str(intensities(pos, :)), mat2str(armedArray(pos, :))), 'updateCoolLED', 'help');
if enable
    pe.ChannelsIntensity = intensities(pos, :);
%     for i = 1 : length(armedArray)
%         if armedArray(pos, i)
%             pe.ChannelsStatus{i} = 'Armed';
%         else
%             pe.ChannelsStatus{i} = 'Off';
%         end
%     end
end

return;

% ------------------------------------------------------------------
function incrementPosition(hObject)

[position, intensities, armedArray] = getLocalBatch(progmanager, hObject, 'positionNumber', 'intensities', 'armedArray');
if size(intensities, 1) ~= size(armedArray, 1)
    error('Size of LED intensities array (%s) does not match size of LED armed array (%s).', mat2str(size(intensities)), mat2str(size(armedArray)));
end
if position == size(intensities, 1)
    position = 1;
else
    position = position + 1;
end
setLocal(progmanager, hObject, 'positionNumber', position);
position_Callback(hObject, [], []);

return;

% ------------------------------------------------------------------
function genericUpdateFcn(hObject, eventdata, handles)

[intensities, enable] = getLocalBatch(progmanager, hObject, 'intensities', 'enable');

if ~isempty(intensities)
    setLocalGh(progmanager, hObject, 'led1Intensity', 'Enable', 'On');
    setLocalGh(progmanager, hObject, 'led2Intensity', 'Enable', 'On');
    setLocalGh(progmanager, hObject, 'led3Intensity', 'Enable', 'On');
    setLocalGh(progmanager, hObject, 'led4Intensity', 'Enable', 'On');

    setLocalGh(progmanager, hObject, 'led1Armed', 'Enable', 'On');
    setLocalGh(progmanager, hObject, 'led2Armed', 'Enable', 'On');
    setLocalGh(progmanager, hObject, 'led3Armed', 'Enable', 'On');
    setLocalGh(progmanager, hObject, 'led4Armed', 'Enable', 'On');
    
    setLocalGh(progmanager, hObject, 'position', 'Enable', 'On');
else
    setLocalGh(progmanager, hObject, 'led1Intensity', 'Enable', 'Off');
    setLocalGh(progmanager, hObject, 'led2Intensity', 'Enable', 'Off');
    setLocalGh(progmanager, hObject, 'led3Intensity', 'Enable', 'Off');
    setLocalGh(progmanager, hObject, 'led4Intensity', 'Enable', 'Off');

    setLocalGh(progmanager, hObject, 'led1Armed', 'Enable', 'Off');
    setLocalGh(progmanager, hObject, 'led2Armed', 'Enable', 'Off');
    setLocalGh(progmanager, hObject, 'led3Armed', 'Enable', 'Off');
    setLocalGh(progmanager, hObject, 'led4Armed', 'Enable', 'Off');

    setLocalGh(progmanager, hObject, 'position', 'Enable', 'Off');
end

if enable
    setLocalGh(progmanager, hObject, 'enable', 'String', 'Off', 'ForegroundColor', [1, 0, 0]);
else
    setLocalGh(progmanager, hObject, 'enable', 'String', 'On', 'ForegroundColor', [0, 0.8, 0]);
end

position_Callback(hObject, eventdata, handles);

return;

% ------------------------------------------------------------------
function genericCloseFcn(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function out = getVersion(hObject, eventdata, handles)

out = 0.1;

return;

% ------------------------------------------------------------------
function genericPreSaveSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPreLoadSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPreLoadMiniSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPostSaveSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPostLoadSettings(hObject, eventdata, handles, varargin)

position_Callback(hObject, eventdata, handles);

return;

% ------------------------------------------------------------------
function genericPostLoadMiniSettings(hObject, eventdata, handles, varargin)
return;

% ------------------------------------------------------------------
function genericPreGetHeader(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPostGetHeader(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericOpenData(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericSaveProgramData(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericSaveProgramDataAs(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPreCacheSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPreCacheMiniSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPostCacheSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericPostCacheMiniSettings(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericCacheOperationBegin(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
function genericCacheOperationComplete(hObject, eventdata, handles)
return;

% ------------------------------------------------------------------
% --- Executes on button press in enable.
function enable_Callback(hObject, eventdata, handles)

genericUpdateFcn(hObject, eventdata, handles);

[pe, enable] = getLocalBatch(progmanager, hObject, 'ledObj', 'enable');
% msgbox(sprintf('Update CoolLED...\nIntensities: %s\nArmed: %s', mat2str(intensities(pos, :)), mat2str(armedArray(pos, :))), 'updateCoolLED', 'help');
if ~enable
    pe.ChannelsIntensity(:) = 0;
%     for i = 1 : 4
%         pe.ChannelsStatus{i} = 'Off';
%     end
end

return;