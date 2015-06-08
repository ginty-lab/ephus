%% ramp and indentation stimuli custom ephus pulses
Fs = 40000;

% make force step series, 7.5 sec in length
steps = signalobject('Name',['igridf'] ,'sampleRate', Fs);
literal(steps, ephusStimuli.makeStepSeries([20,60,180,540,1620,2500],.2,.5,Fs));
lstep = signalobject('Name',['igridl'] ,'sampleRate', Fs);
literal(lstep, ephusStimuli.lpFilterFc(ephusStimuli.makeStep(.25, 7, .25, 9400,Fs), 75));

% make individual force step waves

f20 = signalobject('Name',['f20'] ,'sampleRate', Fs);  
literal(f20,ephusStimuli.lpFilterFc(ephusStimuli.makeStep(.2, .15, .2, 20,Fs), 250));
f60 = signalobject('Name',['f60'] ,'sampleRate', Fs);  
literal(f60,ephusStimuli.lpFilterFc(ephusStimuli.makeStep(.2, .15, .2, 60,Fs), 250));
f180 = signalobject('Name',['f180'] ,'sampleRate', Fs);  
literal(f180,ephusStimuli.lpFilterFc(ephusStimuli.makeStep(.2, .15, .2, 180,Fs), 250));
f540 = signalobject('Name',['f540'] ,'sampleRate', Fs);  
literal(f540,ephusStimuli.lpFilterFc(ephusStimuli.makeStep(.2, .15, .2, 540,Fs), 250));
f1620 = signalobject('Name',['f1620'] ,'sampleRate', Fs);  
literal(f1620,ephusStimuli.lpFilterFc(ephusStimuli.makeStep(.2, .15, .2, 1620,Fs), 250));
lsingle = signalobject('Name',['lsingle'] ,'sampleRate', Fs);
literal(lsingle,ephusStimuli.lpFilterFc(ephusStimuli.makeStep(.1, .4, .1, 9000,Fs), 80));

%save the file as a pulse
destDir = getDefaultCacheDirectory(progmanager, 'myCustomPulses');
if strcmpi(destDir, pwd)
     destDir = getDefaultCacheDirectory(progmanager, 'pulseDir');
end
destDir = uigetdir(destDir, 'Choose a pulseSet for the destination of new pulses.');

setDefaultCacheValue(progmanager, 'myCustomPulses', destDir);

for signal = steps
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = lstep
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = lsingle
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = f20
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = f60
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = f180
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = f540
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = f1620
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = f2500
%    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end