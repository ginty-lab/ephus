% SIGNALOBJECT/signalobject - An object representing a time-series signal.
%
% SYNTAX
%  s = signalobject - Gets an empty signal object.
%  s = signalobject(PROPERTY_NAME, PROPERTY_VALUE, ...) - Allows `set` functionality in the constructor.
%  s2 = signalobject(s1) - Returns a deep copy of s1.
%
% USAGE
%  The true utility of the @signalobject is in calling the `getdata` method, which will return an array of time-series data, that is
%  defined by the object.
%
% STRUCTURE
%  All fields of the @signalobject are readable through the case-insensitive `get` method. However, not all fields may be mutated
%  via the case-insensitive `set` method. Many fields require specific functions to be called, in order to change the field while
%  maintaining object consistency.
%
%  type - Any of the following:
%   'analytic' - A signal defined by a periodic/non-periodic function.
%   'literal' - A signal defined by an arbitrary array of data.
%   'recursive' - A signal composed of other signals, which are combined through mathematical operations.
%   'squarePulseTrain' - This is the equivalent of pulses in the original Physiology software, and is provided for convenience.
%   'digitalPulseTrain' - This is a variant of a squarePulseTrain that will change amplitudes to 1 and offsets to 0, 'or' instead of add, and returns data as type uint32.
%                         See @signalobject/convert2Digital to convert an existing squarePulseTrain into a digitalPulseTrain.
%   'raster' - A 1D raster scan, however this is intended to be used with a pair of @signalobject instances, for a 2D scan (one @signalobject per axis).
%
%  cache - A boolean flag, which determines whether this signal gets cached.
%  eagerCacheGeneration - A boolean flag, which determines when to calculate cached data.
%  length - The amount of available data [seconds]. Values of <0 are considered as infinity.
%  sampleRate - The number of samples per second [Hz].
%  repeatable - A boolean flag, which determines if this signal may be repeated out to infinity.
%  phaseUnits - UNUSED.
%  noPadding - Blocks this object from padding data when insufficient data is available.
%
%  debugMode - A boolean flag, which turns on debugging utilities within the code.
%  plotAnalyticSignalGeneration - Enables plotting of data generation at various stages (has no effect if debugMode is off).
%  eagerWarningMode - Aggressively print warning messages.
%  warnAnalyticPadding - Issue a warning when data padding occurs for analytic signals (overridden by eagerWarningMode).
%  readOnlyFields - A list of case-insensitive field names, which will be locked (if readOnlyFields is one of them, this value is permanently locked).
%  deleteChildrenAutomatically - Boolean: delete any children of this signal, when it is deleted itself. This is off by default.
%  noPadding - Boolean: prevent automatic padding when literal pulses are too short for the requested time.
%
%  signal - The cached/literal data.
%  updated - A flag to signal the need for cache regeneration.
%
%  children - Sub-signals, which will get combined, in a recursive signal.
%  parents - The signals for which this sub-signal is a child.
%  signalPhaseShift - The phase difference, in seconds, between individual children (relative to the origin).
%  method - The mathematical operation used to combine sub-signals.
%  autogenerated - This was generated by a signalobject method, and has only one parent, thus the parent may safely delete it.
%
%  amplitude - The amplitude (in arbitrary units) of this analytic signal.
%  offset - The offset (in amplitude space) of this analytic signal, relative to the origin.
%  phi - The offset (in time space) of this analytic signal, relative to the origin.
%  symmetry - The "leaning" of an analytic signal in time space (for example, the midpoint of a triangle wave).
%  periodic - Flag determining if this analytic signal is periodic.
%   frequency - The frequency of this periodic signal.
%   waveform - The waveform of this periodic signal. Valid waveforms include:
%    sine
%    cosine
%    triangle
%    square
%  equational - Flag determining if this analytic signal is specified by an unbounded Matlab equation on t (t is the time variable) in string format.
%    equation - The Matlab function string that acts on a time vector, f(t).
%  distribution - The statistical distribution of this aperiodic signal (expressed as a probability density function in time).
%   Valid distributions (and their arguments) include:
%    gaussian - (mean, variance)
%    poisson - (lambda)
%    binomial - (N, P)
%    beta - (A, B)
%    chi-squared - (V)
%    non-central chi-squared - (V, delta)
%    discrete uniform - (N)
%    exponential - (mean)
%    f - (v1, v2)
%    non-central f - (nu1, nu2, delta)
%    gamma - (A, B)
%    geometric - (P)
%    lognormal - (mean, variance)
%    negative binomial - (R, P)
%    rayleigh - (B)
%    t - (V)
%    non-central t - (V, delta)
%    weibull - (A, B)
%    hypergeometric - (A, B)
%   See Matlab's documentation on "Probability Distributions" for more details.
%
%  squarePulseTrainNumber - Used for the 'squarePulseTrain' type. The number of square pulses.
%  squarePulseTrainISI - Used for the 'squarePulseTrain' type. The inter-stimulus interval.
%  squarePulseTrainWidth - Used for the 'squarePulseTrain' type. The time-width of each pulse.
%  squarePulseTrainDelay - Used for the 'squarePulseTrain' type. The pre-pulse time delay (ie. time elapsed before inital pulse).
%
%  rasterLinesPerFrame - Used for the 'raster' type. The number of lines per frame.
%  rasterNumberOfFrames - Used for the 'raster' type. The total number of frames.
%  rasterInterFrameInterval - Used for the 'raster' type. The time, in milliseconds, from the start of one frame until the start of the next.
%  rasterAxis - Used for the 'raster' type. This will either be 'fast' or 'slow'.
%  rasterPark - Used for the 'raster' type. The nominal (park) position, when not scanning.
%
%  stepFcnOnsetTimes - Used for the 'stepFcn' type, to indicate the onset time (in seconds) of each step.
%  stepFcnWidths - Used for the 'stepFcn' type, to indicate the width (in seconds) of each step, may be Inf.
%
%  arg1, arg2, arg3 - These arguments to the distribution's probability density function have meanings specific to the distribution. Some distributions will use
%  less than all three arguments, in which case, the remaining arguments are ignored.
%
%  fcn - UNUSED.
%  fcnTakesArgs - UNUSED.
%
% NOTES:
%
%  Pointers - This object uses the pseudo-pointer system (see @daqmanager for more info).
%
% CHANGES:
%  Moved the setting of all the signal specification variables into setDefaultsByType. -- Tim O'Connor 10/22/04 TO102204a
%  Added the warnAnalyticPadding variable. -- Tim O'Connor 2/3/05 TO020305c.
%  Added the noPadding variable. -- Tim O'Connor 2/3/05 TO020305d.
%  Added the autogenerated field, to allow easy memory cleanup for compound signals. -- Tim O'Connor 4/4/05 TO0404050B
%  Added the squarePulseTrain type, to simply port over the parameters from the original Physiology software. -- Tim O'Connor 5/2/05 TO050205A
%  Allowed parameters to get set via the constructor. -- Tim O'Connor 8/9/05 TO080905B
%  I'm not sure, but I think this was wrong. The pointer should go to the end of the signalobject array. -- Tim O'Connor 8/10/05 TO081005D
%  Fixed some shenanigans going on with pointers. See also TO081005D. -- Tim O'Connor TO081105C
%  Optimization(s). Keep the read-only fields in a lower-case list to facilitate compare operations. -- Tim O'Connor 2/27/06 TO022706D
%  Initialized the noPadding field, see TO020305d. -- Tim O'Connor 11/10/06 TO111006A
%  Added a 'raster' type. -- Tim O'Connor 12/08/09 TO120809A
%  Added a version field. -- Tim O'Connor 3/3/10 TO030310B
%  Promoted 'stepFcn' to a full-fledged type. -- Tim O'Connor 6/11/10 TO061110C
%
% Created 8/19/04 - Tim O'Connor
% Copyright - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2004
function this = signalobject(varargin)
global signalobjects;

if isempty(signalobjects)
    signalobjects(1).name = 'Master';
    signalobjects(1).signal = [];
end

%Allow creation of a signal pointer that is just an index into the global signal array.
if length(varargin) == 1 && isnumeric(varargin{1}) && ~isempty(signalobjects(1).signal)
    this.ptr = signalobjects(1).signal(find(signalobjects(1).signal(:, 2) == varargin{1}), 1);
    this.serialized = [];
    this = class(this, 'signalobject');
    fprintf(1, 'Creating @signal object %s --> %s\n', num2str(this.ptr), num2str(indexOf(this)));
    return;
end

%The new pointer.
if isempty(signalobjects(1).signal)
    this.ptr = length(signalobjects) + 1;
else
    this.ptr = max(signalobjects(1).signal(:, 1)) + 1;
end

%Don't overwrite loaded data.
if ~isfield(this, 'serialized')
    this.serialized = [];
end

%TO081105C - Pointer mayhem!
signalobjects(1).signal(size(signalobjects(1).signal, 1) + 1, 1) = this.ptr;
signalobjects(1).signal(size(signalobjects(1).signal, 1), 2) = length(signalobjects) + 1;%TO081005D: This was being set to this.ptr... Is that bad? -- Tim O'Connor 8/10/05
pointer = signalobjects(1).signal(size(signalobjects(1).signal, 1), 2);

%Update timestamps.
signalobjects(pointer).instantiationTime = clock;
signalobjects(pointer).saveTime = -1;
signalobjects(pointer).loadTime = -1;

signalobjects(pointer).version = 2.0;%TO030310B
signalobjects(pointer).name = 1.0;
signalobjects(pointer).type = '';

signalobjects(pointer).name = ['@SignalObject-' num2str(pointer)];

%Options.
signalobjects(pointer).cache = 0;
signalobjects(pointer).eagerCacheGeneration = 0;
signalobjects(pointer).length = -1;
signalobjects(pointer).sampleRate = 1000;
signalobjects(pointer).repeatable = 1;
signalobjects(pointer).phaseUnits = 'seconds';
signalobjects(pointer).readOnlyFields = lower({'instantiationTime', 'saveTime', 'loadTime', 'autogenerated'});%TO022706D
signalobjects(pointer).deleteChildrenAutomatically = 0;
signalobjects(pointer).noPadding = 0;%TO111006A

%Debugging utilities.
signalobjects(pointer).debugMode = 0;
signalobjects(pointer).warnAnalyticPadding = 0;%TO020305c
signalobjects(pointer).eagerWarningMode = 1;
signalobjects(pointer).plotAnalyticSignalGeneration = 0;

%Moved the setting of all the signal specification variables into setDefaultsByType. -- Tim O'Connor TO102204a
setDefaultsByType(this);

this = class(this, 'signalobject');
% fprintf(1, '@signalobject/signalobject: this.ptr=%s, pointer=%s, indexOf=%s, name=''%s''\n%s', num2str(this.ptr), num2str(pointer), num2str(indexOf(this)), signalobjects(pointer).name);
%Make a deep copy.
if ~isempty(varargin)
    if strcmpi(class(varargin{1}), 'signal')
        signalobjects(pointer) = signalobjects(varargin{1}.ptr);
    elseif isstruct(varargin{1})
        signalobjects(pointer) = varargin{1};
    else
        set(this, varargin{:});%TO080905B
    end
end
% fprintf(1, '@signalobject/signalobject: this.ptr=%s, pointer=%s, indexOf=%s, name=''%s''\n%s\n', num2str(this.ptr), num2str(pointer), num2str(indexOf(this)), signalobjects(pointer).name, getStackTraceString);
return;

%Replaced all this with the private method `setDefaultsByType`.
% %Cached or custom-specified data.
% signalobjects(this.ptr).signal = [];
% signalobjects(this.ptr).updated = 0;
% 
% %Recursion - Complex signals (additive or multiplicative).
% signalobjects(this.ptr).children = [];
% signalobjects(this.ptr).parents = [];
% signalobjects(this.ptr).signalPhaseShift = [];
% signalobjects(this.ptr).method = 'add';
% 
% %Analytical specification.
% signalobjects(this.ptr).periodic = 0;
% signalobjects(this.ptr).equational = 0;
%   signalobjects(this.ptr).equation = '';
% signalobjects(this.ptr).amplitude = 0;
% signalobjects(this.ptr).offset = 0;
% signalobjects(this.ptr).phi = 0;
% signalobjects(this.ptr).symmetry = 0;
%  %Periodic
%  signalobjects(this.ptr).waveform = '';
%  signalobjects(this.ptr).frequency = 0;
%  %Aperiodic
%  signalobjects(this.ptr).distributional = 0;
%  signalobjects(this.ptr).distribution = '';
%  %The meaning of these arguments depends on the distribution.
%  %For example, in a 'gaussian' distribution, arg1 is the mean, arg2 is the variance, and arg3 is ignored.
%  %A number of distributions depend only on arg1 (ie. 'poisson'), in which case arg2 (or greater) is ignored.
%  signalobjects(this.ptr).arg1 = 0;
%  signalobjects(this.ptr).arg2 = 1;
%  signalobjects(this.ptr).arg3 = 0;
% 
% %Callback specification.
% signalobjects(this.ptr).fcn = {};
% signalobjects(this.ptr).fcnTakesArgs = {};


% %Should this use substructures to group fields? Or, is it better to keep it all on one level, making for a simple get/set method?
% %Options.
% signalobjects(this.ptr).options.cache = 0;
% signalobjects(this.ptr).options.eagerCacheGeneration = 0;
% signalobjects(this.ptr).options.length = 0;
% signalobjects(this.ptr).options.sampleRate = 1000;
% signalobjects(this.ptr).options.repeatable = 1;
% 
% %Cached or custom-specified data.
% signalobjects(this.ptr).numericSpecification.signal = [];
% 
% %Recursion - Complex signals (additive or multiplicative).
% signalobjects(this.ptr).recursion.signals = [];
% signalobjects(this.ptr).recursion.signalPhaseShift = [];
% signalobjects(this.ptr).recursion.method = 'add';
% 
% %Analytical specification.
% signalobjects(this.ptr).analyticSpec.type = '';
% signalobjects(this.ptr).analyticSpec.frequency = 0;
% signalobjects(this.ptr).analyticSpec.amplitude = 0;
% signalobjects(this.ptr).analyticSpec.offset = 0;
% signalobjects(this.ptr).analyticSpec.phi = 0;
% signalobjects(this.ptr).analyticSpec.symmetry = 0;
% 
% %Callback specification.
% signalobjects(this.ptr).fcn = {};
% signalobjects(this.ptr).fcnTakesArgs = {};