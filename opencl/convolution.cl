#define DATA_TYPE unsigned char

// Converts an rgb value to YUV
uint3 rgb2YUV(int r, int g, int b)
{
    uint3 YUV = (0.299*r + 0.587*g + 0.114*b,
                 (b-YUV.x)*0.565,
                 (r-YUV.x)*0.713);
    return YUV;
}

float rgb2Y(DATA_TYPE r, DATA_TYPE g, DATA_TYPE b)
{
    float Y = (float)(0.299)*(float)r + (float)(0.587)*(float)g + (float)(0.114)*(float)b;
    return Y;
}

// Put pixel in a BGR image type
void putPixel ( __global DATA_TYPE *image,
                int src_step, int channel,
                int x, int y,
                unsigned char r, unsigned char g, unsigned char b)
{
    int idx = (y * src_step) + (x*channel);
    image [idx] = b;
    image [idx+1] = g;
    image [idx+2] = r;
}

uint3 getPixel ( __constant DATA_TYPE *image,
                int src_step, int channel,
                int x, int y)
{
    int idx = (y * src_step) + (x * channel);
    return ( /*b = */ image [idx],
              /*g = */ image [idx+1],
              /*r = */ image [idx+2]);
}

#define GHOST_THRESHOLD -100
bool isGhost (
    int x, int y,
    __constant DATA_TYPE *depth,
    int depth_step, int depth_channels,
    int width, int height)
{
    if (x - 1 < 0 || x + 1 >= width) return 0;
    if (y - 1 < 0 || y + 1 >= height) return 0;

    float SUM = 0;
    for (int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++ )
        {
            int neighbor_idx = (y + j)* depth_step + (x + i) * depth_channels;
            DATA_TYPE b = depth[neighbor_idx];
            DATA_TYPE g = depth[neighbor_idx+1];
            DATA_TYPE r = depth[neighbor_idx+2];
            float D = rgb2Y(r, g, b);
            SUM += D;
        }
    }

    int idx = y * depth_step + x * depth_channels;
    DATA_TYPE b = depth[idx];
    DATA_TYPE g = depth[idx+1];
    DATA_TYPE r = depth[idx+2];
    float D = rgb2Y(r, g, b);

    if (SUM - (9.0 * D) > GHOST_THRESHOLD)
        return 0;

    return 1;
}

#define WITH_LOCK 1

__kernel void dibr (
       __constant DATA_TYPE *src, /* Can we change that for a uchar3 ? */
       __constant DATA_TYPE  *depth,
       __global DATA_TYPE *out,
       __global DATA_TYPE  *depth_out,
#if WITH_LOCK
       volatile __global int *depth_mutex,
#endif
       __global DATA_TYPE *mask,
       int rows, int cols,                      // We can pre-compile this values
       int src_step, int out_step, int channel, // We can pre-compile this values
       int mask_step,                           // We can pre-compile this values
       __constant int *depth_shift_table_lookup,
       int S)
{
        const int x = get_global_id(0);
        const int y = get_global_id(1);

//for (int x = 0; x < cols; x ++)
//{
        int idx = (y*src_step) + (x*channel);

        // if ( isGhost (x, y, depth, src_step, channel, cols, rows) ) // Should not project ghost pixels
        //    continue;

        DATA_TYPE b = depth[idx];
        DATA_TYPE g = depth[idx + 1];
        DATA_TYPE r = depth[idx + 2];
        int Y, U, V;

        float D = rgb2Y(r, g, b);
        int shift = depth_shift_table_lookup [ (int)D];

        b = src [idx];
        g = src [idx+1];
        r = src [idx+2];
        S = 20;

#if 1
            if( (x + S - shift) < cols && (x + S - shift) >= 0)
            {
                int newidx = (y  * out_step) + (x + S - shift) * channel;
                int newidx_mask =  y * mask_step + (x + S - shift);
#if WITH_LOCK
                int processed = 0;
                while (!processed)
                {
                    if (atomic_cmpxchg (depth_mutex + (y * 2 * cols + x + S - shift), 0, 1) == 0) // got the lock
                    {
                        int previousDepthOut = depth_out[newidx];
                        if ( mask [newidx_mask] != '1' ||
                             D > previousDepthOut ) // I need to process
                        {
#endif
                            int currentMask = mask[newidx_mask];
                            out [newidx] = b;
                            out [newidx+1] = g;
                            out [newidx+2] = r;

                            mask [newidx_mask] = '1';
                            depth_out[newidx] = (char)D;
                             /* else if ((int) D == currentDepthOut && newidx > idx)
                             {
                                 out [newidx] = b;
                                 out [newidx+1] = g;
                                 out [newidx+2] = r;

                                 mask [newidx_mask] = '1';
                                 depth_out[newidx] = (int)D;
                             } */
#if WITH_LOCK
                        }
                        processed = 1;
                        // free the lock
                        atomic_xchg (depth_mutex + (y * 2 * cols + x + S - shift), 0);
                    }
                    barrier(CLK_GLOBAL_MEM_FENCE);
                }
#endif

            }
/*
            if( x + shift - S >= 0 && x + shift - S < cols )
            {
                int newidx = (y  * out_step) + (x + cols + shift - S) * channel;
                int newidx_mask =  y * mask_step + (x + shift - S) + cols;
                int currentDepthOut = depth_out [newidx];
                int currentMask = mask[newidx_mask];

                if( currentMask != '1' ||
                    (int) D >= depth_out [newidx] )
                {
                    out [newidx] = b;
                    out [newidx+1] = g;
                    out [newidx+2] = r;

                    mask [newidx_mask] = '1';
                    depth_out[newidx] = (int)D;
                }

                if ((int) D == currentDepthOut && newidx > idx)
                {
                    out [newidx] = b;
                    out [newidx+1] = g;
                    out [newidx+2] = r;

                    mask [newidx_mask] = '1';
                    depth_out[newidx] = (int)D;
                }
            } */
#else
            //shift *= 2;
            if( x + S - shift < cols)
            {
                int newidx_mask =  y * mask_step + (x + S - shift);
                int newidx = (y  * out_step) + (x + S - shift) * channel;

                if(depth_out [newidx] <= D)
                {
                    out [newidx] = b;
                    out [newidx + 1] = g;
                    out [newidx + 2] = r;

                    depth_out[newidx] = (int)D;
                }

                mask [newidx_mask] = '1';
            }

            int newidx = (y * out_step) + (x + cols ) * channel;
            out [newidx] = b;
            out [newidx+1] = g;
            out [newidx+2] = r;

            newidx = y * mask_step + x + cols;
            mask [newidx] = '1';
#endif
//}

}

__kernel void hole_filling (
        __global DATA_TYPE *src,
        __global DATA_TYPE *out,
        __global DATA_TYPE *depthOut,
       __global DATA_TYPE *mask,
        int rows, int cols,
        int src_step, int out_step, int channel, int mask_step,
        int INTERPOLATION_HALF_SIZE_WINDOW)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    int background = 255;

    int idxMask = y * mask_step + x;
    if(mask[idxMask] != '1') // is hole
    {
        float sumB = 0, sumG = 0, sumR = 0, Y = 0;
        float total = 0;

        // search for background depth
        for (int i = -INTERPOLATION_HALF_SIZE_WINDOW; i <= INTERPOLATION_HALF_SIZE_WINDOW; i++)
        {
            for (int j = -INTERPOLATION_HALF_SIZE_WINDOW; j <= INTERPOLATION_HALF_SIZE_WINDOW; j++)
            {
                if (x + i >= 0 && x + i < 2 * rows
                    && y + j >= 0 && y + j <= cols)
                {
                    int idxOut1 = (y + j) * out_step + (x + i) * channel;
                    int idxMask1 = (y + j) * mask_step + x + i;

                    if(mask[idxMask1] == '1') // its not a hole
                        if (depthOut[idxOut1] < background)
                            background = depthOut[idxOut1];
                }
             }
        }

        // Do interpolation only with foreground objects
        for (int i = -INTERPOLATION_HALF_SIZE_WINDOW; i <= INTERPOLATION_HALF_SIZE_WINDOW; i++)
        {
            for (int j = -INTERPOLATION_HALF_SIZE_WINDOW; j <= INTERPOLATION_HALF_SIZE_WINDOW; j++)
            {
                if (x + i >= 0 && x + i < 2 * rows
                    && y + j >= 0 && y + j <= cols)
                {
                    int idxOut1 = (y + j) * out_step + (x + i) * channel;
                    int idxMask1 = (y + j) * mask_step + x + i;

                    if(mask[idxMask1] == '1' && depthOut[idxOut1] == background) // its not a hole and is background
                    {
                        DATA_TYPE r, g, b;
                        b = out [idxOut1];
                        g = out [idxOut1 + 1];
                        r = out [idxOut1 + 2];

                        sumB += (int) b;
                        sumG += (int) g;
                        sumR += (int) r;

                        total += 1.0;
                    }
                }
            }
        }

        int idxOut = y * out_step + x * channel;
        out[idxOut] = (DATA_TYPE)(sumB/total);
        out[idxOut+1] = (DATA_TYPE)(sumG/total);
        out[idxOut+2] = (DATA_TYPE)(sumR/total);
    }
}

__kernel void convolute (
       __global DATA_TYPE *src,
       __global DATA_TYPE *out,
       int rows, int cols,
       int src_step, int channel,
       int FILTER_HALF_SIZE,
       __constant float *filter)
{
        const int x = get_global_id(1);
        const int y = get_global_id(0);

        if( x >= rows || y >= cols)
            return;

        int idx = (y*src_step)+(x*channel);

        float sumR = 0.0;
        float sumG = 0.0;
        float sumB = 0.0;

#if 1
        if ( (x - FILTER_HALF_SIZE) < 0 || (y - FILTER_HALF_SIZE < 0 ))
        {
            out[idx] = src[idx];
            out[idx+1] = src[idx+1];
            out[idx+2] = src[idx+2];
        }
        else
        {
            for (int i = -FILTER_HALF_SIZE; i <= FILTER_HALF_SIZE; i++)
            {
                for(int j = -FILTER_HALF_SIZE; j <= FILTER_HALF_SIZE; j++)
                {
                    int idxTmp = (y + j) * src_step + (x + i) * channel;
                    int idxFilter = (i + FILTER_HALF_SIZE) * 3 + j + FILTER_HALF_SIZE;

                    sumB += src[idxTmp] * filter[idxFilter];
                    sumG += src[idxTmp+1] * filter[idxFilter];
                    sumR += src[idxTmp+2] * filter[idxFilter];
                }
            }
        }

        if (sumB > 255) sumB = 255;
        if (sumG > 255) sumG = 255;
        if (sumR > 255) sumR = 255;

        out[idx] = sumB;
        out[idx+1] = sumG;
        out[idx+2] = sumR;
#else
        out[idx] = src[idx];
        out[idx+1] = src[idx+1];
        out[idx+2] = src[idx+2];
#endif
}
