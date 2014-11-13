classdef ephusStimuli
    %UNTITLED7 Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        destDir;
        signals;
        signalObj;
    end
    
    methods
        function object = ephusStimuli(destDir,signals, setName)
           if(destDir == '' || exist(destDir) ~= 7)
              destDir = uigetdir(destDir, 'Choose a pulseSet for the destination of new pulses.'); 
           else
            object.destDir = destDir;
           end
           
           object.signalObj = signalobject('Name',[setName] ,'sampleRate', 10000);
           setDefaultCacheValue(progmanager, 'myCustomPulses', destDir);
        end
        
        function saveSignals(ES)
            literal(ES.signalObj, ES.signals);
            for signal = ES.signals
                saveCompatible(fullfile(ES.destDir, [get(signal, 'Name') '.signal']), 'signal', '-mat');
            end 
        end
        
        function addSignal(ES,signal)
            ES.signals = cat(1,ES.signals, signal);
        end
    end
    
    methods(Static)
      
      function stp = makeStep(base, length, trailing, amplitude,Fs)
        stp(1:(base + length + trailing) *Fs) = 0;
        stp(round(base * Fs):(round(base * Fs)+round(length * Fs))) = amplitude;
      end

      function filtered = lpFilter(signal, Fcut)
        [num,den]= besself(5,Fcut);
        [numd,dend]=bilinear(num,den,10000);
        filtered = filtfilt(numd,dend,signal);
      end

    function [fRamps,lRamps] = makeRampSeries(iri, amplitude, slopes,Fs)
        %ramps = ephusStimuli.makeRampSeries(.2,1,[1,2,4,8,16],10000);
     fRamps=[];
     for i=[1:max(size(slopes))]
          fRamps = cat(2,fRamps,zeros(1,iri*Fs));
           fRamps = cat(2,fRamps, ephusStimuli.makeRamp(amplitude/slopes(i),slopes(i),Fs)); 
     end     
      fRamps = cat(2,fRamps,zeros(1,iri*Fs));  
      lRamps = fRamps;
      lRamps(lRamps > 0)= amplitude;
      lRamps = ephusStimuli.lpFilter(lRamps);
      fRamps = ephusStimuli.lpFilter(fRamps);
    end

    function steps = makeStepSeries(amplitudes, length, isi,Fs) 
     %steps = ephusStimuli.makeStepSeries([0.06,.2,.6, 1.8,5.4],.5, .25,10000);
     steps=[];
     for i=(1:max(size(amplitudes)))
        steps = cat(2,steps,ephusStimuli.makeStep(isi, length, isi, amplitudes(i),Fs));
     end
     steps = ephusStimuli.lpFilter(steps,200);
    end

    function ramp = makeRamp(length, slope,Fs)
        %time = linspace(0, length, length*Fs);;
        ramp = linspace(0, length, length*Fs);
        ramp = ramp * slope;
    end

    function [fWNstep,lWNstep] = filteredWNStep(base, length, trailing, amplitude, Fs)
        % filteredWNStep(.25,10,.6,100,10000)
        noisePower = 1;
        step(1,[1:base*Fs]) = 0;
        step(1,[(base*Fs + 1): (base + length)*Fs]) = amplitude;
        step(1,[(base*Fs + 1): (base + length)*Fs]) = awgn(step(1,[(base*Fs + 1): (base + length)*Fs]),noisePower);
        step(1,[((base + length)*Fs+1):(base + length+trailing)*Fs]) = 0;
        lWNstep = step;
        lWNstep(lWNstep > 0) = amplitude;
        fWNstep = step;
        lWNstep = ephusStimuli.lpFilter(lWNstep);
        fWNstep = ephusStimuli.lpFilter(fWNstep);
    end
    
    end
    
end

