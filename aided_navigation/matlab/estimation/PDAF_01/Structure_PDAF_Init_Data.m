function AllTheData = Structure_PDAF_Init_Data(Par)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Structure_PDAF_Init_Data - initializes data for multiple
% tracking models
% Input:
%   Par         - parameters of the algorithm
% Output:
%   AllTheData  - all the data
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parameters
if ~isfield(Par,'TrajIndex') | ~isfield(Par,'Nv') | ~isfield(Par,'PointNum') | ...
   ~isfield(Par,'NaNDensity'),  
        error('Field is missing');
 end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Generate trajectories and clutter
[y,t,dT] = Generate2DTrajectories(Par.TrajIndex(1),Par.dT,Par.Time);
yc = rand(size(y,1),Par.PointNum,size(y,2));

% check MaxPointNum more then trajectories
if Par.PointNum < Par.TrajNum,
    disp('There are more trajectories then points');
    Par.PointNum = Par.TrajNum+1;
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% !!! Random permutation ???
RandPermTraj = randperm(Par.PointNum);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% init.
for i = 1:Par.TrajNum,
    
    ytmp = Generate2DTrajectories(Par.TrajIndex(i),Par.dT,Par.Time);
    yc(:,RandPermTraj(i),:) = ytmp;
    
end;

yc = yc+randn(size(yc)).*Par.Nv;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Generate missing  points
miss_ind = find(rand(size(yc)) < Par.NaNDensity);
yc(miss_ind)    = NaN;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% save
AllTheData      = yc;

