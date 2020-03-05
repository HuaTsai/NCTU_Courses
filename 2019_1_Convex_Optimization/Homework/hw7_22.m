% file: hw7_2.m

P = [13 12 -2; 12 17 6; -2 6 12];
q = [-22 -14.5 13]';
r = 1;
A = [1 0 0; 0 1 0; 0 0 1; -1 0 0; 0 -1 0; 0 0 -1];
b = [1 1 1 1 1 1]';
[m, n] = size(A);
maxiters = 200;
alpha = 0.01;
beta = 0.5;
nttol = 1e-6;
tol = 1e-3;
mu = 20;
t = 1;
x = [0 0 0]';
inniters = [];
gaps = [];
for i = 1:maxiters
    d = b - A * x;
    val = t * (.5 * x' * P * x + q' * x + r) - sum(log(d));
    g = t * (P * x + q) + A' * (1 ./ d);
    H = t * P + A' * diag(1 ./ d .^ 2) * A;
    xnt = -H \ g;
    fprime = g' * xnt;
    s = 1;
    dd = -A * xnt;
    while (min(d + s * dd) <= 0)
        s = beta * s;
    end
    while (t * (.5 * (x + s * xnt)' * P * (x + s * xnt) + q' * (x + s * xnt) + r) - sum(log(d + s * dd)) >= val + alpha * s * fprime)
        s = beta * s;
    end
    x = x + s * xnt;
    if ((-fprime / 2) < nttol)
        gap = m / t;
        inniters = [inniters, i];
        gaps = [gaps gap];
        if (gap < tol)
            break;
        end
        t = mu * t;
    end
    disp(['x = [ ', sprintf('%f ', x), '], val = ', num2str(((1/2) * x' * P * x + q' * x + r), '%f')]);
end
inniters = [inniters, i];
gaps = [gaps gap];
figure(1)
iters1 = [];
gaps1 = [];
for i = 1:length(gaps)-1
    iters1 = [iters1 inniters(i)-1 inniters(i+1)-1];  
    gaps1 = [gaps1 gaps(i) gaps(i)]; 
end;
iters1 = [iters1 inniters(length(gaps))-1];
gaps1 = [gaps1 gaps(length(gaps))]; 
semilogy(iters1, gaps1);
axis([0 40 1e-6 1e2]);
xlabel('Newton iterations');  ylabel('duality gap');
