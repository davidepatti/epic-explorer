% function f=rdist(o, s, radius)
function f=rdist(o, s, radius)

min1 = min([o(:,1);s(:,1)]);
min2 = min([o(:,2);s(:,2)]);
max1 = max([o(:,1);s(:,1)]);
max2 = max([o(:,2);s(:,2)]);

o(:,1) = (o(:,1)-min1)/(max1-min1);
o(:,2) = (o(:,2)-min2)/(max2-min2);
s(:,1) = (s(:,1)-min1)/(max1-min1);
s(:,2) = (s(:,2)-min2)/(max2-min2);


f = 0;
for i=1:size(o,1),
	x1 = o(i,1);
	y1 = o(i,2);
	for j=1:size(s,1),
		x2 = s(j,1);
		y2 = s(j,2);
		
		dist = sqrt( (x1-x2)^2 + (y1-y2)^2 );
                if (dist <= radius),
			f = f + 1;
			break;
		end
	end
end

f = f/size(o,1);
