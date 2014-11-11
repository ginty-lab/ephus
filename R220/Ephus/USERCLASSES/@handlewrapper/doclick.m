% @handleWrapper/doclick - A callthrough to a handle's `doclick` method.
%
%  SYNTAX
%   See functions for graphics handles.
%
%  CHANGES
%
% Created 12/7/05 - Tim O'Connor
% Copyright - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2005
function varargout = doclick(this, varargin)

varargout = doclick(this.hObject, varargin{:});

return;