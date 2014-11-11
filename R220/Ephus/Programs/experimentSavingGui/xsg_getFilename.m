% xsg_getFilename - Get the fully qualified filename, sans extension.
%
%  SYNTAX
%   filename = xsg_getFileName
%
%  CHANGES
%   TO042106C - Allow options for augmenting the path with the experiment # and set ID. -- Tim O'Connor 4/21/06
%   TO091310A - Implemented externalFilenameGenerator, for custom file name generation solutions (requested for the GECI project). -- Tim O'Connor 9/13/10
%
% Created 5/19/05 - Tim O'Connor
% Copyright - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2005
function filename = xsg_getFilename

hObject = xsg_getHandle;

directory = xsg_getPath;
[initials, experimentNumber, setID, acquisitionNumber, externalGenerator] = getLocalBatch(progmanager, hObject, ...
    'initials', 'experimentNumber', 'setID', 'acquisitionNumber', 'externalFilenameGenerator');
fname = '';

if ~isempty(externalGenerator)
    try
        switch(lower(class(externalGenerator)))
            case 'char'
                fprintf(1, 'XSG: Calling external filename generator ''%s''...\n', externalGenerator);
                eval(['fname = ' externalGenerator ';']);
            case 'function_handle'
                fprintf(1, 'XSG: Calling external filename generator ''%s''...\n', func2str(externalGenerator));
                fname = feval(externalGenerator);
            case 'cell'
                if ~isempty(externalGenerator) && strcmpi(class(externalGenerator{1}), 'function_handle')
                    fprintf(1, 'XSG: Calling external filename generator ''%s''...\n', func2str(externalGenerator{1}));
                    fname = feval(externalGenerator{:});
                else
                    fprintf(2, 'XSG: Invalid externalFilenameGenerator, switching back to default file naming scheme.');
                end
        end
    catch
        fprintf(2, 'XSG: Failed to retrieve custom filename -\n%s\n', getLastErrorStack);
    end
end

if isempty(fname)
    fname = [initials experimentNumber setID acquisitionNumber];
end

filename = fullfile(directory, fname);

return;