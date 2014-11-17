function [Lstep, fsteps] = indentStim()

fConv = 50; %mN/V
eConv = 1000 ; %V/V
forces = [4,12,36,108]./ fConv;
forces = forces .* eConv;
lVoltage =9500;
fsteps = ephusStimuli.makeStepSeries(forces,.08, .08,40000);
Lstep = ephusStimuli.lpFilter(ephusStimuli.makeStep(.03, .85, .05, lVoltage,40000), 50);


end