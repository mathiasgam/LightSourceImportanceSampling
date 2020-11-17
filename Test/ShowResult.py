from matplotlib.pyplot import axis
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from pandas.core.arrays.sparse import dtype
import math

def load_image_from_csv(filepath:str):
    data = pd.read_csv(filepath).to_numpy()
    #print(np.shape(data))
    img = np.zeros((512,512,3), dtype=np.float)

    for y in range(0,511):
        for x in range(0, 511):
            pixel = data[(511-y)*512 + x][:]
            img[y,x] = pixel

    return img

def mse(imageA, imageB):
	# the 'Mean Squared Error' between the two images is the
	# sum of the squared difference between the two images;
	# NOTE: the two images must have the same dimension
	err = np.sum((imageA.astype("float") - imageB.astype("float")) ** 2)
	err /= float(imageA.shape[0] * imageA.shape[1])
	
	# return the MSE, the lower the error, the more "similar"
	# the two images are
	return err

def show_diff_image(imgA, imgB):
    diff = np.abs(img_ref - img_test)
    error = diff[:,:,0] + diff[:,:,1] + diff[:,:,2]

    error = np.clip(error,0.0,1.0)
    
    plt.imshow(error, cmap="nipy_spectral")
    plt.colorbar()
    plt.show()

def show_img(img):
    plt.imshow(img)
    plt.show()

def exposure(img, exposure):
    # reinhard tone mapping
    return img / (img+exposure)

def gamma(img, gamma):
    return np.power(img, 1.0 / gamma)

def color_correct(img, _gamma, _exposure):
    return gamma(exposure(img, _exposure), _gamma)

img_test = load_image_from_csv("Test.csv")
img_ref = load_image_from_csv("Ref.csv")

print("MSE: {}".format(mse(img_test, img_ref)))

show_diff_image(img_test, img_ref)

show_img(color_correct(img_test, 2.2, 1.0))



#plt.hist(img.ravel(), bins=256, range=(0.0, 1.0), fc='k', ec='k')

#a11=nArray.reshape(512,512)


#plt.show()





