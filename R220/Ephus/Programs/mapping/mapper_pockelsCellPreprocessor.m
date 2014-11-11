% mapper_pockelsCellPreprocessor - An @AOMUX preprocessor function for the mapper to control a Pockels cell.
%
% SYNTAX
%  mapper_pockelsCellPreprocessor(hObject, data)
%    hObject - The mapper program handle.
%    data - The actual data to preprocess.
%
% USAGE
%
% NOTES
%  This is only intended as a temporary implementation of a Pockels cell preprocessor. It may,
%  however, grow into a more permanent function, at which time it should be extracted
%  from the mapper.
%
%  The control signal is expected to vary between 0 and 100 (ie. the units are percent).
%
% CHANGES
%  TO031306C: Scaling 0-.100 V, convert that into normalized units. -- Tim O'Connor 3/13/06
%  TO102508H: Handle the 'coeffs' variable's potential non-existence. -- Tim O'Connor 10/25/08
%  TO021510E: Make the Pockels cell optional. -- Tim O'Connor 2/15/10
%  BS061211: Instead of polynomial fit, use a lookup table to transform the
%  input data to appropriate Pockels command voltages. Optimized by only
%  looking up unique levels, of which there are few in typical
%  step-function pulses.
%  BSTO100511A - Adjusted the conditions, to pick up on an uncalibrated Pockels cell with the new calibration routine. -- Ben Suter/Tim O'Connor 10/5/11
%
% Created 3/10/06 Tim O'Connor
% Copyright - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006
function preprocessed = mapper_pockelsCellPreprocessor(hObject, data)
% fprintf(1, 'mapper_pockelsCellPreprocessor: InitialRange = [%s %s].\n', num2str(min(data)), num2str(max(data)));
try
    %TO102508H - Just checking for empty below is not enough, because this call may fail, so try/catch it. -- Tim O'Connor 10/25/08
    coeffs = getLocal(progmanager, hObject, 'coeffs');
    pockels_lookup = getLocal(progmanager, hObject, 'pockels_lookup');
catch
    coeffs = [];
    pockels_lookup = [];
end
% if isempty(coeffs)
if isempty(coeffs) && (isempty(pockels_lookup) || any(isnan(pockels_lookup.transmitted_power))) % BS061211 %BSTO100511A
    preprocessed = zeros(size(data));
    warning('Pockels cell appears to be uncalibrated.');
    return;
end

% Data comes in as Volts, so a value of 100 A.U. from pulse editor, which
% is interpreted as 100 mV, arrives here as 0.1 V. Multiplying the input by
% 10 ensures that a pulseEditor value of 100 A.U. maps to a value of 1.0
% here, and thus that the 0 - 100 % intention maps to 0.0 to 1.0 here.
% 2011-06-11, Ben Suter
data = 10 * data;%TO031306C

% %Apply the 3rd degree polynomial fit.
% try%LTP021208 Added try catch with Tim over the phone 
%     basis = cat(2, ones(size(data)), data, data.^2, data.^3);
%     preprocessed = basis * coeffs;
% catch
%     % BS061111 Display message when enter this condition, and slight
%     % indentation change.
%     disp('Exception caught while preprocessing Pockels command, so concatenating along dim 1 now');
%     basis = cat(1, ones(size(data)), data, data.^2, data.^3)';
%     preprocessed = basis * coeffs;
% end

% Use the lookup table instead for better fit:
% data is the desired transmitted power, ranging from 0 to 1
% I don't know how to vectorize this, so using a loop for now.
% Can optimize by calculating only for unique values, which helps 
% because most pulses are step functions. This optimization reduced
% the time required to process a 0.4 ms pulse at 10 kHz substantially, 
% measured as a 211x improvement (from about 80 ms down to 0.4 ms).
% A second optimization, namely not interpolating when an exact
% match is found may also speed things up a bit, but mainly it avoids some
% possible error conditions.
preprocessed = [];
try
    levels = unique(data);
    for k=1:numel(levels)
        x1 = max(pockels_lookup.transmitted_power(pockels_lookup.transmitted_power <= levels(k)));
        if x1 == levels(k)
            % No need to interpolate if matches exactly
            idx1 = find(pockels_lookup.transmitted_power == x1, 1, 'first');
            preprocessed(data == levels(k)) = pockels_lookup.command_voltage(idx1);
        else
            % Interpolate
            x2 = min(pockels_lookup.transmitted_power(pockels_lookup.transmitted_power >= levels(k)));
            idx1 = find(pockels_lookup.transmitted_power == x1, 1, 'last'); % in case there are multiple with same reading
            idx2 = find(pockels_lookup.transmitted_power == x2, 1, 'first'); % ditto
            y1 = pockels_lookup.command_voltage(idx1);
            y2 = pockels_lookup.command_voltage(idx2);
            preprocessed(data == levels(k)) = y1 + (y2-y1)*(levels(k)-x1)/(x2-x1);
        end
    end    
catch ME
    warning('Error while preprocessing pockels command. Setting all command values to zero.');
    preprocessed = zeros(size(data));
end

% Debugging figure
% figure; hold on; plot(data, 'ok'); plot(preprocessed, 'ob'); 

%Watch out for over/under voltages and NaNs.
indicesOfNaNs = find(preprocessed == NaN);
overVoltage = find(preprocessed > getLocal(progmanager, hObject, 'modulatorMax'));%For UV, no modulation beyond 1V is necessary. %TO021510E - The range is now configurable.
underVoltage = find(preprocessed < getLocal(progmanager, hObject, 'modulatorMin'));%All modulation is positive. %TO021510E - Not anymore...
if ~isempty(overVoltage)
    fprintf(2, 'Warning: mapper_pockelsCellPreprocessor - Pockels cell calibration or control signal may be invalid. Voltages were found to be out of range - OVER_VOLTAGE\n%s', getStackTraceString);
    figure;
    plot(1:length(data), data, ':v', 1:length(preprocessed), preprocessed, ':o', 1:length(overVoltage), preprocessed(overVoltage), 'x');
    title('Pockels Cell OverVoltage');
    legend('Preprocessed data', 'Overvoltage samples');
%     preprocessed(overVoltage) = 2;
    preprocessed(overVoltage) = getLocal(progmanager, hObject, 'modulatorMax'); % 2011-05-02 Ben Suter -- correct using the configured limit, not hard-coded 2
    fprintf(2, 'To set "fake" pockels cell calibration data, run: setGlobal(progmanager, ''coeffs'', ''mapper'', ''mapper'', [0 0.1 0.1 0.1]'')\n');
end
if ~isempty(underVoltage)
    fprintf(2, 'Warning: mapper_pockelsCellPreprocessor - Pockels cell calibration or control signal may be invalid. Voltages were found to be out of range - UNDER_VOLTAGE\n%s', getStackTraceString);
    figure;
    plot(1:length(data), data, ':v', 1:length(preprocessed), preprocessed, ':o', 1:length(underVoltage), preprocessed(underVoltage), 'x');
    title('Pockels Cell UnderVoltage');
    legend('Preprocessed data', 'Undervoltage samples');
%     preprocessed(underVoltage) = 0;
    preprocessed(underVoltage) = getLocal(progmanager, hObject, 'modulatorMin'); % 2011-05-02 Ben Suter -- correct using the configured limit, not hard-coded 0
    fprintf(2, 'To set "fake" pockels cell calibration data, run: setGlobal(progmanager, ''coeffs'', ''mapper'', ''mapper'', [0 0.1 0.1 0.1]'')\n');
end
if ~isempty(indicesOfNaNs)
    fprintf(2, 'Warning: mapper_pockelsCellPreprocessor - Pockels cell calibration or control signal may be invalid. Voltages were found to be out of range - NaN\n%s', getStackTraceString);
    figure;
    plot(1:length(data), data, ':v', 1:length(preprocessed), preprocessed, ':o', 1:length(indicesOfNaNs), preprocessed(indicesOfNaNs), 'x');
    title('Pockels Cell NaN');
    legend('Preprocessed data', 'NaN samples');
    preprocessed(indicesOfNaNs) = 0; % 2011-05-02 Ben Suter -- leaving as is, but not sure whether good to set these to 0 (would min be better, I think not, hmm)
    fprintf(2, 'To set "fake" pockels cell calibration data, run: setGlobal(progmanager, ''coeffs'', ''mapper'', ''mapper'', [0 0.1 0.1 0.1]'')\n');
end
% fprintf(1, 'mapper_pockelsCellPreprocessor: FinalRange = [%s %s].\n', num2str(min(data)), num2str(max(data)));
return;