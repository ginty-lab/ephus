Fs = 40000;
% make ramps
fsig = signalobject('Name',['framps'] ,'sampleRate', Fs);
lsig = signalobject('Name',['lramps'] ,'sampleRate', Fs);
[lr,fr] = indentStim(); 
literal(fsig, fr);
literal(lsig, lr);
% make ramps
fsig = signalobject('Name',['fr'] ,'sampleRate', Fs);
lsig = signalobject('Name',['lr'] ,'sampleRate', Fs);
literal(fsig, fr);
literal(lsig, lr);




destDir = 'C:\Program Files (x86)\Ephus\settings\BPL\EphusPulses\indent';
exist(destDir, 'dir')
setDefaultCacheValue(progmanager, 'myCustomPulses', destDir);

for signal = fsig
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end

for signal = lsig
    saveCompatible(fullfile(destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
end
