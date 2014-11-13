function [Lstep, fsteps] = indentStim()

fConv = 50; %mN/V
eConv = 1000 ; %V/V
forces = [2,8,32,128]./ fConv;
forces = forces .* eConv;
lVoltage =9500;
fsteps = ephusStimuli.makeStepSeries(forces,.08, .07,40000);
Lstep = ephusStimuli.lpFilter(ephusStimuli.makeStep(.04, .85, .05, lVoltage,40000), 50);


end