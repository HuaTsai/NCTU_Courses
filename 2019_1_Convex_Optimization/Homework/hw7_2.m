% file: hw7_2.m

P = [13 12 -2; 12 17 6; -2 6 12];
q = [-22 -14.5 13]';
r = 1;
A = [1 0 0; 0 1 0; 0 0 1; -1 0 0; 0 -1 0; 0 0 -1];
b = [1 1 1 1 1 1]';
[m, n] = size(A);
maxiters = 100;
alpha = 0.01;
beta = 0.5;
nttol = 1e-7;
mu = 2;

t = 1;
x = [0 0 0]';
gaps = [];
inniters = [];
disp('Iter(0): find central point of t = 1, x = x0')
for i = 1:maxiters
    d = b - A * x;
    val = t * ((1/2) * x' * P * x + q' * x + r) - sum(log(d));
    g = P * x + q + A' * (1 ./ d);
    H = A' * diag(1 ./ (d .^ 2)) * A;
    xnt = -H \ g;  
    fprime = g' * xnt;
    s = 1;
    while true
        newx = x + s * xnt;
        if (min(b - A * newx) >= 0)
            newval = t * ((1/2) * newx' * P * newx + q' * newx + r) - sum(log(b - A * newx));
            if (newval < val + alpha * s * fprime)
                break;
            end
        end
        s = beta * s;
    end
    x = x + s * xnt;
    disp(['  inneriter(', sprintf('%02d', i), '): x = [ ', sprintf('%f ', x), '], val = ', num2str(((1/2) * x' * P * x + q' * x + r), '%f')]);
    if ((-fprime / 2) < nttol)
        break;
    end
end
t = mu * t;
for j = 1:maxiters
    disp(['Iter(', num2str(i), '): find central point of t = ', sprintf('%f', t)]);
    d = b - A * x;
    val = t * ((1/2) * x' * P * x + q' * x + r) - sum(log(d));
    g = P * x + q + A' * (1 ./ d);
    H = A' * diag(1 ./ (d .^ 2)) * A;
    xnt = -H \ g;  
    fprime = g' * xnt;
    s = 1;
    while true
        newx = x + s * xnt;
        if (min(b - A * newx) >= 0)
            newval = t * ((1/2) * newx' * P * newx + q' * newx + r) - sum(log(b - A * newx));
            if (newval < val + alpha * s * fprime)
                break;
            end
        end
        s = beta * s;
    end
    x = x + s * xnt;
    disp(['  inneriter(', sprintf('%02d', j), '): x = [ ', sprintf('%f ', x), '], val = ', num2str(((1/2) * x' * P * x + q' * x + r), '%f')]);
    if ((-fprime / 2) < nttol)
        t = min(t * mu, (m + 1) / 1e-5);
    end
end
disp(['  inneriter(', sprintf('%02d', j), '): x = [ ', sprintf('%f ', x), '], val = ', num2str(((1/2) * x' * P * x + q' * x + r), '%f')]);
