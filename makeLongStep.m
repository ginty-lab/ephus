%% ramp and indentation stimuli custom ephus pulses
Fs = 40000;

% make steps
longStepL = signalobject('Name',['steps'] ,'sampleRate', 40000);
literal(longStepL, ephusStimuli.makeStep(1, 11, 1, 5000,Fs));
longStepF = signalobject('Name',['steps'] ,'sampleRate', 40000);
literal(longStepF, ephusStimuli.makeStep(1.5,10,1.5, 100, Fs));


%save the file as a pulse
destDir = getDefaultCacheDirectory(progmanager, 'myCustomPulses');
if strcmpi(destDir, pwd)
     destDir = getDefaultCacheDirectory(progmanager, 'pulseDir');
end
destDir = uigetdir(destDir, 'Choose a pulseSet for the destination of new pulses.');

setDefaultCacheValue(progmanager, 'myCustomPulses', destDir);

for signal = longStepL
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = longStepF
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

