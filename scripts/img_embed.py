# img_embed.py
# image must be 8bpp

from PIL import Image
import numpy as np


def img_to_bitmap(img):
    arr = np.array(img)

    r,g,b = np.split(arr,3,axis=2)
    r=r.reshape(-1)
    g=r.reshape(-1)
    b=r.reshape(-1)

    bitmap = list(map(lambda x: 0.299*x[0]+0.587*x[1]+0.114*x[2], zip(r,g,b)))
    bitmap = np.array(bitmap).reshape([arr.shape[0], arr.shape[1]])
    return bitmap

if __name__ == "__main__":
    import sys
    import os.path

    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} [image]')
        exit()

    img = Image.open(sys.argv[1])
    out_base = os.path.splitext(os.path.basename(sys.argv[1]))[0]
    out_filename = out_base + '_8bpp.h'

    out_file = open(out_filename, "w")

    bitmap = img_to_bitmap(img)
    print(bitmap.shape)

    w = bitmap.shape[1]
    h = bitmap.shape[0]
    bitmap = bitmap.reshape([w * h])
                    
    out_file.write(f'const unsigned char {out_base}_8bpp[{w * h}] = {{\n')

    for i in range(len(bitmap)):
        bitmap[i] = max(255 - bitmap[i], 0)
    bitmap = bitmap.astype(np.uint8)

    #print(bitmap)

    row = 16
    for i in range(0, len(bitmap), row):
        out_file.write('    ')
        for j in range(row):
            x = bitmap[i + j]
            out_file.write(f'{x:#0{4}x}, ')
        out_file.write('\n')

    out_file.write('};\n\n');

    out_file.write( f'const struct Image8bpp img_{out_base} = {{\n' \
                    f'    {w}, // width\n' \
                    f'    {h}, // height\n' \
                    f'    {out_base}_8bpp // data\n' \
                    '};\n' );

    out_file.close()

    print(f'Saved {out_filename}')


