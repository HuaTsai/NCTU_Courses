% file: hw7_1.m

A = [4 3 2 1 0; 1 1 1 1 1];
b = [20 15]';
[m, n] = size(A);
maxiters = 50;
alpha = 0.01;
beta = 0.5;
nttol = 1e-7;

% (a) Feasible start Newton method
x = [1 2 3 4 5]';
disp('(a) Feasible start Newton method');
disp('    Iterate optimal points and values:');
disp(['           x(0) = [ ', sprintf('%f ', x), ']']);
disp(['      optval(0) = ', num2str(sum(x.*log(x)), '%f')]);
for i = 1:maxiters
    g = log(x) + 1;
    H = diag(1 ./ x);
    KKTleft = [H A'; A zeros(m)];
    KKTright = [-g; zeros(m, 1)];
    KKTv = KKTleft \ KKTright;
    xnt = KKTv(1:n);
    fprime = g' * xnt;
    if ((-fprime / 2) < nttol)
        break;
    end
    t = 1;
    val = sum(x .* log(x));
    while true
        newx = x + t * xnt;
        if (min(newx) >= 0)
            newval = sum((newx) .* log(newx));
            if (newval < val + alpha * t * fprime)
                break;
            end
        end
        t = beta * t;
    end
    x = x + t * xnt;
    disp(['           x(', int2str(i), ') = [ ', sprintf('%f ', x), ']']);
    disp(['      optval(', int2str(i), ') = ', num2str(sum(x.*log(x)), '%f')]);
end

% (b) Infeasible start Newton method with (1, 2, 3, 4, 5)
x = [1 2 3 4 5]';
nu = zeros(m, 1);
disp('(b) Infeasible start Newton method with x = (1, 2, 3, 4, 5)');
disp('    Iterate optimal points and values:');
disp(['           x(0) = [ ', sprintf('%f ', x), ']']);
disp(['      optval(0) = ', num2str(sum(x.*log(x)), '%f')]);
for i = 1:maxiters
    g = log(x) + 1;
    H = diag(1 ./ x);
    r = [g + A' * nu; A * x - b];
    KKTleft = [H A'; A zeros(m)];
    KKTright = -r;
    KKTv = KKTleft \ KKTright;
    xnt = KKTv(1:n);
    nunt = KKTv(n+1:n+m);
    t = 1;
    while true
        newx = x + t * xnt;
        newnu = nu + t * nunt;
        if (min(newx) >= 0)
            newr = [(log(newx) + 1) + A' * (newnu); A * (newx) - b];
            if norm(newr) < (1 - alpha) * norm(r)
                break;
            end
        end
        t = beta * t;
    end
    x = x + t * xnt;
    nu = nu + t * nunt;
    disp(['           x(', int2str(i), ') = [ ', sprintf('%f ', x), ']']);
    disp(['      optval(', int2str(i), ') = ', num2str(sum(x.*log(x)), '%f')]);
    if (norm(newr) < nttol)
        break;
    end
end

% (b) Infeasible start Newton method with (5, 2, 3, 4, 5)
x = [5 2 3 4 5]';
nu = zeros(m, 1);
disp('(b) Infeasible start Newton method with x = (5, 2, 3, 4, 5)');
disp('    Iterate optimal points and values:');
disp(['           x(0) = [ ', sprintf('%f ', x), ']']);
disp(['      optval(0) = ', num2str(sum(x.*log(x)), '%f')]);
for i = 1:maxiters
    g = log(x) + 1;
    H = diag(1 ./ x);
    r = [g + A' * nu; A * x - b];
    KKTleft = [H A'; A zeros(m)];
    KKTright = -r;
    KKTv = KKTleft \ KKTright;
    xnt = KKTv(1:n);
    nunt = KKTv(n+1:n+m);
    t = 1;
    while true
        newx = x + t * xnt;
        newnu = nu + t * nunt;
        if (min(newx) >= 0)
            newr = [(log(newx) + 1) + A' * (newnu); A * (newx) - b];
            if norm(newr) < (1 - alpha) * norm(r)
                break;
            end
        end
        t = beta * t;
    end
    x = x + t * xnt;
    nu = nu + t * nunt;
    disp(['           x(', int2str(i), ') = [ ', sprintf('%f ', x), ']']);
    disp(['      optval(', int2str(i), ') = ', num2str(sum(x.*log(x)), '%f')]);
    if (norm(newr) < nttol)
        break;
    end
end
