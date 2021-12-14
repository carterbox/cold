#include "core.h"



void
footprint(int siz, float tau, float dec, float* signal)
{
    siz--;
    int threshold = tau * siz / 2;
    int x;
    for (x = 0; x < siz; x++)
    {   
        if (x < threshold)
        {
            signal[x] = 0.5 - 0.5 * cosf(2 * PI * x / (tau * siz));
        } 
        else if (x >= threshold && x <= siz - threshold)
        {
            signal[x] = 1;
        } 
        else if (x > siz - threshold)
        {
            signal[x] = 0.5 - 0.5 * cosf(2 * PI * (siz - x) / (tau * siz));
        }
    }
    float sum = 0;
    float thickness = 1;
    for (int x = 0; x < siz; x++)
    {   
        signal[x] /= expf(-siz + thickness * dec * dec);
        thickness++;
        sum += signal[x];
    }
    for (int x = 0; x < siz; x++)
    {   
        signal[x] /= sum;
    }
}


void
convolve(
    float* sig, int ns, float* kernel, int nk,  float* res)
{
    int begin, end;
    int l, m;
    int first = (nk - 1) / 2;
    for (l = 0; l < ns; l++)
    {   
        begin = MAX(l - nk + 1 + first, 0);
        end = MIN(l + 1 + first, ns);
        
        for (m = begin; m < end; m++)
        {
            res[l] += sig[m] * kernel[l - m + first];
        }
    }
}


void
copyarr(float* arr, int size, float* copy)
{
    for (int l = 0; l < size; l++)
    {   
        copy[l] = arr[l];
    }
}


void
maxarr(float* arr, int size, float *maxval)
{
    for (int l = 1; l < size; l++) 
    {
        if (arr[l] > *maxval)
        {
            *maxval = arr[l];
        } 
    }
}


void
minarr(float* arr, int size, float *minval)
{
    for (int l = 1; l < size; l++) 
    {
        if (arr[l] < *minval)
        {
            *minval = arr[l];
        } 
    }
}


void
minmaxarr(float* arr, int size, float *minval, float *maxval)
{
    for (int l = 1; l < size; l++) 
    {
        if (arr[l] > *maxval)
        {
            *maxval = arr[l];
        } 
        else if (arr[l] < *minval)
        {
            *minval = arr[l];
        }
    }
}


void
normalize(float* arr, int size)
{
    float minval = arr[0];
    minarr(arr, size, &minval);
    float stdmin = 2 * sqrtf(minval);
    for (int l = 0; l < size; l++)
    {   
        arr[l] = arr[l] - (minval + stdmin);
    }
    float maxval = arr[0];
    maxarr(arr, size, &maxval);
    float stdmax = 2 * sqrtf(maxval);
    for (int l = 0; l < size; l++)
    {   
        arr[l] = arr[l] / (maxval - stdmax);
    }
}


int
recposlsqr(
    float* data, int dx, 
    float* mask, int mx, 
    int cor, int pos, int sig, float tau, float dec)
{
    float cost;
    float minimum;
    float temp;
    int k, l;
    int costsize = mx - dx + 1;
    int totsiz = sig + cor;
    float minval;
    float maxval;
    float* _data = (float*) calloc(dx, sizeof(float));
    float* _mask = (float*) calloc(mx, sizeof(float));
    float* kernel = (float*) calloc(totsiz, sizeof(float));

    copyarr(data, dx, _data);
    normalize(_data, dx);
    footprint(totsiz, tau, dec, kernel);
    convolve(mask, mx, kernel, totsiz, _mask);
    minval = _mask[0];
    maxval = _mask[0];
    minmaxarr(_mask, mx, &minval, &maxval);

    pos = 0;
    minimum = 1e+32f;
    for (k = 0; k < costsize; k++) // costsize
    {

        cost = 0.0f;
        for (l = 0; l < dx; l++)
        {   
            temp = (_mask[k + l] - minval) / (maxval - minval);
            cost += powf(_data[l] - temp, 2);
        }
        if (cost < minimum) 
        {
            minimum = cost;
            pos = k;      
        }
    }
    free (_mask);
    free (_data);
    free (kernel);
    return pos;
}


int
recsiglsqr(
    float* data, int dx, 
    float* mask, int mx,
    int cor, int pos, int sig, float tau, float dec)
{
    float cost;
    float minimum;
    float temp;
    int k, l;
    int low = MAX(sig - 10 * 2, 2);
    int high = sig + 10 * 2;
    float minval;
    float maxval;
    float* kernel;
    float* _mask;
    float* _data = (float*) calloc(dx, sizeof(float));

    copyarr(data, dx, _data);
    normalize(_data, dx);

    minimum = 1e+32f;
    for (k = low; k < high; k++) // costsize
    {   
        int totsiz = k + cor;
        kernel = (float*) calloc(totsiz, sizeof(float));
        footprint(totsiz, tau, dec, kernel);
        _mask = (float*) calloc(mx, sizeof(float));
        convolve(mask, mx, kernel, totsiz, _mask);
        minval = _mask[0];
        maxval = _mask[0];
        minmaxarr(_mask, mx, &minval, &maxval);

        cost = 0.0f;
        for (l = 0; l < dx; l++)
        {   
            temp = (_mask[pos + l] - minval) / (maxval - minval);
            cost += powf(_data[l] - temp, 2);
        }
        if (cost < minimum) 
        {
            minimum = cost;
            sig = k;
        }
        free (_mask);
        free (kernel);
    }
    free (_data);
    return sig;
}


float
rectaulsqr(
    float* data, int dx, 
    float* mask, int mx,
    int cor, int pos, int sig, float tau, float dec)
{
    float cost;
    float minimum;
    float temp;
    float minval;
    float maxval;
    float low = MAX(tau - 0.1, 0);
    float high = MIN(tau + 0.1, 1);
    float _tau;
    int k, l;
    float* kernel;
    float* _mask;
    float* _data = (float*) calloc(dx, sizeof(float));

    copyarr(data, dx, _data);
    normalize(_data, dx);

    minimum = 1e+32f;
    for (k = 0; k < 20; k++) // costsize
    {   
        _tau = low + k * 0.01;
        if (_tau > high)
        {
            _tau = high;
        }
        kernel = (float*) calloc(sig, sizeof(float));
        footprint(sig, _tau, dec, kernel);
        _mask = (float*) calloc(mx, sizeof(float));
        convolve(mask, mx, kernel, sig, _mask);
        minval = _mask[0];
        maxval = _mask[0];
        minmaxarr(_mask, mx, &minval, &maxval);

        cost = 0.0f;
        for (l = 0; l < dx; l++)
        {   
            temp = (_mask[pos + l] - minval) / (maxval - minval);
            cost += powf(_data[l] - temp, 2);
        }
        if (cost < minimum) 
        {
            minimum = cost;
            tau = _tau;
        }
        free (_mask);
        free (kernel);
    }
    free (_data);
    return tau;
}


float
recdeclsqr(
    float* data, int dx, 
    float* mask, int mx,
    int cor, int pos, int sig, float tau, float dec)
{
    float cost;
    float minimum;
    float temp;
    float minval;
    float maxval;
    float low = MAX(dec - 0.1, 0);
    float _dec;
    int k, l;
    float* kernel;
    float* _mask;
    float* _data = (float*) calloc(dx, sizeof(float));

    copyarr(data, dx, _data);
    normalize(_data, dx);

    minimum = 1e+32f;
    for (k = 0; k < 20; k++) // costsize
    {   
        _dec = low + k * 0.01;
        kernel = (float*) calloc(sig, sizeof(float));
        footprint(sig, tau, _dec, kernel);
        _mask = (float*) calloc(mx, sizeof(float));
        convolve(mask, mx, kernel, sig, _mask);
        minval = _mask[0];
        maxval = _mask[0];
        minmaxarr(_mask, mx, &minval, &maxval);

        cost = 0.0f;
        for (l = 0; l < dx; l++)
        {   
            temp = (_mask[pos + l] - minval) / (maxval - minval);
            cost += powf(_data[l] - temp, 2);
        }
        if (cost < minimum) 
        {
            minimum = cost;
            dec = _dec;
        }
        free (_mask);
        free (kernel);
    }
    free (_data);
    return dec;
}
