# 瀵煎叆opencv
import cv2
import numpy as np
import serial
import struct
from pyzbar.pyzbar import decode
import math

g_num = [0, 0, 0, 0, 0, 0, 0]
'''
g_R_l_hsv = [135, 129, 92]
g_R_h_hsv = [180, 255, 255]

g_G_l_hsv = [59, 60, 80]
g_G_h_hsv = [88, 255, 255]

g_B_l_hsv = [91, 128, 76]
g_B_h_hsv = [132, 255, 255]
'''
g_R_l_hsv = [148, 45, 141]
g_R_h_hsv = [180, 255, 255]

g_G_l_hsv = [52, 33, 99]
g_G_h_hsv = [84, 255, 255]

g_B_l_hsv = [77, 79, 100]
g_B_h_hsv = [136, 255, 255]

g_zb_num = [0, 0, 0, 0, 0, 0]

g_good = []

g_zx_x = 320
g_zx_y = 200
g_zqfw = 100

g_flog01 = 0

# 鍒涘缓鎽勫儚澶村璞?
cap0 = cv2.VideoCapture(0)
# cap1 = cv2.VideoCapture(2)
cap0.set(3, 640)  # 瀹藉害
cap0.set(4, 480)  # 楂樺害
# cap1.set(3, 640)  # 瀹藉害
# cap1.set(4, 480)  # 楂樺害
# print(cap0.get(10))  # 浜害
#  set(10, 2)  # 浜害
#  print(cap0.get(10))  # 浜害
#  鍒涘缓涓插彛閫氫俊瀵硅薄
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.05)


#  鑹插潡璇嗗埆
def sksb(frame, g_l_hsv, g_h_hsv):
    # 澶嶅埗涓€浠藉師鍥?
    frame_copy = frame.copy()
    # 杞寲涓篐SV
    hsv = cv2.cvtColor(frame_copy, cv2.COLOR_BGR2HSV)
    # 浜屽€煎寲澶勭悊
    lower = np.array(g_l_hsv)
    higher = np.array(g_h_hsv)
    mask = cv2.inRange(hsv, lower, higher)
    cv2.imshow('mask', mask)
    # 鑾峰彇褰㈡€佸鍗风Н鏍?
    # kernel1 = cv2.getStructuringElement(cv2.MORPH_OPEN, (5, 5))
    # 鐢ㄥ紑杩愮畻婊ゆ尝
    # mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel1, iterations=1)
    # cv2.imshow('mask1', mask)
    # 鑾峰彇褰㈡€佸鍗风Н鏍?
    kernel2 = cv2.getStructuringElement(cv2.MORPH_OPEN, (5, 5))
    # 鐢ㄩ棴杩愮畻婊ゆ尝
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel2, iterations=5)
    # 鏄剧ず婊ゆ尝鍚庣収鐗?
    cv2.imshow('mask2', mask)
    contours, hierarchy = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    #  姹囧埗杞粨
    if len(contours) != 0:
        #  鑾峰彇杞粨鐨勫鎺ュ渾
        area = []
        # 鎵惧埌鏈€澶х殑杞粨
        for k in range(len(contours)):
            area.append(cv2.contourArea(contours[k]))
        max_idx = np.argmax(np.array(area))
        # print(max_idx)
        cv2.drawContours(frame_copy, contours, max_idx, (0, 255, 0), 2)
        center, radius = cv2.minEnclosingCircle(contours[max_idx])
        if (radius * radius * 3 > 1000):
            # 杈撳嚭鍦嗗績
            # print(center,radius)
            # 缁樺埗澶栨帴鍦?
            cv2.circle(frame_copy, (int(center[0]), int(center[1])), int(radius), (255, 255, 255), 2)
            # 鏄剧ず鐢昏疆寤撳拰澶栨帴鍦嗗悗鐓х墖
            cv2.imshow('frame_copy', frame_copy)
            return center
        else:
            return (0, 0)
    return (0, 0)


# 璇嗗埆鐗╂枡鍧愭爣锛堜笉甯﹂鑹诧級
def wlsb(frame):
    center = sksb(frame, g_R_l_hsv, g_R_h_hsv)
    if ((center[0] + center[1]) != 0):
        return center
    else:
        center = sksb(frame, g_G_l_hsv, g_G_h_hsv)
        if ((center[0] + center[1]) != 0):
            return center
        else:
            center = sksb(frame, g_B_l_hsv, g_B_h_hsv)
            if ((center[0] + center[1]) != 0):
                return center
    return (0, 0)


# 璇嗗埆鐗╂枡鍧愭爣锛堝甫棰滆壊锛?
def wlyssb(flag, frame, x, y, zqfw):
    if flag == b'\x01':
        center = sksb(frame, g_R_l_hsv, g_R_h_hsv)
        if (((center[0] + center[1]) != 0) and (abs(center[0] - x) < zqfw) and (abs(center[1] - y) < zqfw)):
            return b'\x01'
    if flag == b'\x02':
        center = sksb(frame, g_G_l_hsv, g_G_h_hsv)
        if (((center[0] + center[1]) != 0) and (abs(center[0] - x) < zqfw) and (abs(center[1] - y) < zqfw)):
            return b'\x02'
    if flag == b'\x03':
        center = sksb(frame, g_B_l_hsv, g_B_h_hsv)
        if (((center[0] + center[1]) != 0) and (abs(center[0] - x) < zqfw) and (abs(center[1] - y) < zqfw)):
            return b'\x03'
    print((abs(center[0] - x) < 10), (abs(center[1] - y) < 10))
    return b'\x00'


# 璇嗗埆鍦嗙幆
def sbyh(frame):
    # 澶嶅埗涓€浠藉師鍥?
    frame_copy = frame.copy()
    # 鏌ユ壘杈规骞朵簩鍊煎寲
    img = cv2.Canny(frame_copy, 150, 200)
    cv2.imshow('img0', img)
    # 鑾峰彇褰㈡€佸鍗风Н鏍?
    kernel2 = cv2.getStructuringElement(cv2.MORPH_OPEN, (5, 5))
    # 鐢ㄩ棴杩愮畻婊ゆ尝
    img = cv2.morphologyEx(img, cv2.MORPH_CLOSE, kernel2, iterations=6)
    cv2.imshow('img1', img)
    # 鑾峰彇褰㈡€佸鍗风Н鏍?
    kernel1 = cv2.getStructuringElement(cv2.MORPH_OPEN, (5, 5))
    # 鐢ㄥ紑杩愮畻婊ゆ尝
    img = cv2.morphologyEx(img, cv2.MORPH_OPEN, kernel1, iterations=3)
    cv2.imshow('img2', img)
    # 鏌ユ壘杞粨
    contours, hierarchy = cv2.findContours(img, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    # 姹囧埗杞粨
    if len(contours) != 0:
        # 鑾峰彇杞粨鐨勫鎺ュ渾
        area = []
        # 鎵惧埌鏈€澶х殑杞粨
        for k in range(len(contours)):
            area.append(cv2.contourArea(contours[k]))
        max_idx = np.argmax(np.array(area))
        cv2.drawContours(frame_copy, contours, max_idx, (0, 255, 0), 2)
        center, radius = cv2.minEnclosingCircle(contours[max_idx])
        if (radius * radius * 3 > 1000):
            # 杈撳嚭鍦嗗績
            # print(center)
            # 缁樺埗澶栨帴鍦?
            cv2.circle(frame_copy, (int(center[0]), int(center[1])), int(radius), (255, 255, 0), 2)
            cv2.circle(frame_copy, (int(center[0]), int(center[1])), 1, (255, 0, 0), 2)
            # 鏄剧ず鐢昏疆寤撳拰澶栨帴鍦嗗悗鐓х墖
            cv2.imshow('frame_copy', frame_copy)
            return center
        else:
            return (0, 0)
    return (0, 0)


# 璇嗗埆浜岀淮鐮?
def ewmsb(frame):
    # 澶嶅埗涓€浠藉師鍥?
    frame_copy = frame.copy()

    barcodes = decode(frame_copy)

    for barcode in barcodes:
        (x, y, w, h) = barcode.rect

        cv2.rectangle(frame_copy, (x, y), (x + w, y + h), (0, 255, 0), 2)

        barcode_data = barcode.data.decode("utf-8")
        barcode_type = barcode.type

        cv2.putText(frame_copy, barcode_data, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        cv2.imshow("Barcode Scanner", frame_copy)
        # print(barcode_data)
        if (len(barcode_data) == 7):
            return barcode_data
    return ''


# 鑾峰彇浜岀淮鐮佷俊鎭爣蹇椾綅
def get_num(frame):
    strnum = ewmsb(frame)
    # print(len(strnum))
    if (len(strnum) == 0):
        return 0
    i = 0
    for k in strnum:
        if (i != 3):
            g_num[i] = int(k)
        i += 1
    return 1


# 姣旇緝涓績鐐瑰潗鏍?
def bj(zb_nums, yz):
    zb_x_num = (zb_nums[0] + zb_nums[2] + zb_nums[4]) // 3
    zb_y_num = (zb_nums[1] + zb_nums[3] + zb_nums[5]) // 3
    # print((abs(zb_nums[0]-zb_x_num)<yz),(abs(zb_nums[2]-zb_x_num)<yz),(abs(zb_nums[4]-zb_x_num)<yz),(abs(zb_nums[1]-zb_y_num)<yz),(abs(zb_nums[3]-zb_y_num)<yz),(abs(zb_nums[5]-zb_y_num)<yz))
    if ((abs(zb_nums[0] - zb_x_num) < yz) and (abs(zb_nums[2] - zb_x_num) < yz) and (
            abs(zb_nums[4] - zb_x_num) < yz) and (abs(zb_nums[1] - zb_y_num) < yz) and (
            abs(zb_nums[3] - zb_y_num) < yz) and (abs(zb_nums[5] - zb_y_num) < yz)):
        return 1
    return 0


# 鍙戦€佷簩缁寸爜鏁版嵁
def fsewm():
    temp = struct.pack("<BBBBBBBBB", 0XFF, 0x01, g_num[0], g_num[1], g_num[2], g_num[4], g_num[5], g_num[6],
                       (0xFF + 0x01 + g_num[0] + g_num[1] + g_num[2] + g_num[4] + g_num[5] + g_num[6]) % 0x100)
    ser.write(temp)
    # print(temp)


# 鍙戠敓鐗╂枡鍧愭爣
def fswlzb(x, y):
    x = int(x)
    y = int(y)
    temp = struct.pack("<BBHHB", 0XFF, 0x02, x, y,
                       (0xFF + 0x02 + (x & 0xFF) + (x // 0x100) + (y & 0xFF) + (y // 0x100)) % 0x100)
    print(x, y)
    ser.write(temp)


# 鍙戦€佸渾鐜潗鏍?
def fsyhzb(x, y):
    x = int(x)
    y = int(y)
    print(x, y)
    temp = struct.pack("<BBHHB", 0XFF, 0x03, x, y,
                       (0xFF + 0x03 + (x & 0xFF) + (x // 0x100) + (y & 0xFF) + (y // 0x100)) % 0x100)
    ser.write(temp)


# 鍙戦€佹姄鍙栨寚浠?
def fszl():
    temp = struct.pack("<BBBB", 0XFF, 0x04, 0X01, 0X04)
    ser.write(temp)


while (1):
    # 璇诲彇鐓х墖
    ret0, frame0 = cap0.read()
    if ret0 == True:
        # 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
        cv2.imshow('frame0', frame0)
    # ret1,frame1 = cap1.read()
    # if ret1 == True:
    # 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
    # cv2.imshow('frame1',frame1)

    if True:
        num = ser.read()
        # print(num)
        if num == b'\xFF':
            g_good = []
            g_good.append(num)
            num = ser.read()
            g_good.append(num)
            num = ser.read()
            g_good.append(num)
            # print(num)
            # print('******')
            print(g_good)
            # print(good)
            if len(g_good) == 3:
                if g_good[0] == b'\xFF':
                    '''
                    if (g_good[1] == b'\x01')and(g_flog01 == 0):
                        g_flog01 = 1
                        while(1):
                            # 璇诲彇鐓х墖
                            ret1, frame1 = cap1.read()
                            if ret1 == True:
                                # 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
                                cv2.imshow('frame1', frame1)
                            if(get_num(frame1)):
                                fsewm()
                                #cap1.release()
                                i = 0
                                while(i<4):
									# 璇诲彇鐓х墖
                                    ret1, frame1 = cap1.read()
                                    if ret1 == True:
									# 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
                                        cv2.imshow('frame1', frame1)
                                        if(get_num(frame1)):
                                            fsewm()
                                            i+=1
                                            print(i)
                                    key = cv2.waitKey(10)
                                cap1.release()
                                break
                            key = cv2.waitKey(10)
                    '''
                    if g_good[1] == b'\x02':
                        while (1):
                            # 璇诲彇鐓х墖
                            ret0, frame0 = cap0.read()
                            if ret0 == True:
                                # 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
                                # frame0 = frame0[0:380,0:640]
                                cv2.imshow('frame0', frame0)
                                center = wlsb(frame0)
                                if ((center[0] + center[1]) != 0):
                                    i = 0
                                    g_zb_num[i * 2] = center[0]
                                    g_zb_num[(i * 2) + 1] = center[1]
                                    while (i < 2):
                                        ret0, frame0 = cap0.read()
                                        if ret0 == True:
                                            # 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
                                            cv2.imshow('frame0', frame0)
                                            center = wlsb(frame0)
                                            if ((center[0] + center[1]) != 0):
                                                i += 1
                                                g_zb_num[i * 2] = center[0]
                                                g_zb_num[(i * 2) + 1] = center[1]
                                            if (i == 2):
                                                if (bj(g_zb_num, 3)):
                                                    fswlzb(g_zb_num[4], g_zb_num[5])
                                            key = cv2.waitKey(10)
                                key = cv2.waitKey(10)
                            num = ser.read()
                            # print(num)
                            if num == b'\xFF':
                                g_good = []
                                g_good.append(num)
                                num = ser.read()
                                g_good.append(num)
                                num = ser.read()
                                g_good.append(num)
                                # print(num)
                                # print('******')
                                print(g_good)
                                if len(g_good) == 3:
                                    if g_good[0] == b'\xFF':
                                        if g_good[1] != b'\x02':
                                            break

                    if g_good[1] == b'\x03':
                        while (1):
                            # 璇诲彇鐓х墖
                            ret, frame = cap0.read()
                            if ret == True:
                                # 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
                                cv2.imshow('frame', frame)
                            # frame = frame[0:350,100:540]
                            center = sbyh(frame)
                            if ((center[0] + center[1]) != 0):
                                fsyhzb(center[0], center[1])
                            num = ser.read()
                            # print(num)
                            if num == b'\xFF':
                                g_good = []
                                g_good.append(num)
                                num = ser.read()
                                g_good.append(num)
                                num = ser.read()
                                g_good.append(num)
                                # print(num)
                                # print('******')
                                print(g_good)
                                if len(g_good) == 3:
                                    if g_good[0] == b'\xFF':
                                        if g_good[1] != b'\x03':
                                            break
                            key = cv2.waitKey(10)
                    if g_good[1] == b'\x04':
                        if (g_good[2] != b'\xFF'):
                            while (1):
                                # 璇诲彇鐓х墖
                                ret0, frame0 = cap0.read()
                                if ret0 == True:
                                    # 濡傛灉璇诲彇鎴愬姛鏄剧ず鐓х墖
                                    cv2.imshow('frame0', frame0)
                                    flog = wlyssb(g_good[2], frame0, g_zx_x, g_zx_y, g_zqfw)
                                    print(flog, g_good[2])
                                    if (flog == g_good[2]):
                                        fszl()
                                        break
                                key = cv2.waitKey(10)
                                num = ser.read()
                                # print(num)
                                if num == b'\xFF':
                                    g_good = []
                                    g_good.append(num)
                                    num = ser.read()
                                    g_good.append(num)
                                    num = ser.read()
                                    g_good.append(num)
                                    # print(num)
                                    # print('******')
                                    print(g_good)
                                    if len(g_good) == 3:
                                        if g_good[0] == b'\xFF':
                                            if g_good[1] != b'\x04':
                                                break
    # 绛夊緟鎸夐敭鎸変笅鐨勬椂闂翠负10ms
    key = cv2.waitKey(10)
    # 濡傛灉鎸変笅鎸夐敭骞朵负q閿紝鍒犻櫎鍚嶅瓧涓?window 鐨勭獥鍙?
    if key == ord('q'):
        cv2.destroyAllWindows()
        cap0.release()
        exit(0)

















