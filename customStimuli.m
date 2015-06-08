%% ramp and indentation stimuli custom ephus pulses
Fs = 10000;
% make ramps
framps = signalobject('Name',['framps'] ,'sampleRate', Fs);
lramps = signalobject('Name',['lramps'] ,'sampleRate', Fs);
[fr,lr] = ephusStimuli.makeRampSeries(.2,500,[400,800,1600,3200,6400],10000)
literal(framps, fr);
literal(lramps, lr);

% make steps
steps = signalobject('Name',['steps'] ,'sampleRate', 10000);
literal(steps, ephusStimuli.makeStepSeries([60,180,540, 1620,4860],.5, .25,Fs));


% make WNSteps
base = .25;
length = 5;
trailing = .75;
amplitude = 1000;
[fWN,lWN] = ephusStimuli.filteredWNStep(base, length, trailing, amplitude, Fs);
fWNObj = signalobject('Name',['fWN'] ,'sampleRate', Fs);
literal(fWNObj, fWN);
lWNObj = signalobject('Name',['lWN'] ,'sampleRate', Fs);
literal(lWNObj,lWN);




%save the file as a pulse
destDir = getDefaultCacheDirectory(progmanager, 'myCustomPulses');
if strcmpi(destDir, pwd)
     destDir = getDefaultCacheDirectory(progmanager, 'pulseDir');
end
destDir = uigetdir(destDir, 'Choose a pulseSet for the destination of new pulses.');

setDefaultCacheValue(progmanager, 'myCustomPulses', destDir);

for signal = framps
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = lramps
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = steps
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = fWNObj
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = lWNObj
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end
