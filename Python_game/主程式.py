
import pyautogui as py
import time, snp

py.PAUSE = 0.5

# 1. 打開瀏覽器+最大化

py.hotkey('win','r')
py.press('shift') #換輸入法
py.typewrite('chrome')
py.press('enter')
py.hotkey('win','up')


# 2. 點擊網址列

py.click(286,68)

# 3. 輸入網址

py.press('shift')
py.typewrite('https://gamekuo.com/html5/3963_math-genius-games')
py.press('enter') 

# 4. 頁面移動到正常操作範圍，點擊開始

py.moveTo(1104,906,7)
py.scroll(-310)
py.click(1162,912)
py.click(758,718)
py.PAUSE = 0

# 5. 讀取數字、判斷

for w in range(3):
    time1=time.clock()
    while(1):
        numA=[None,None]
        numB=None
        numC=[None,None]
        a=0
        point=[557,659,920,1028]
        
        number=['0.png','1.png','2.png','3.png','4.png','5.png','6.png','7.png','8.png','9.png']
        time.sleep(0.65)
        for x in number:
            pA=[0]
            for p in snp.locateAllOnScreen(x,threshold=0.95,region=(530,485,590,90)):
                if pA[0]-5 < p[0] < pA[0]+5:
                    continue
                if p[0]<point[0]:
                    numA[0]=a
                    pA[0]=p[0]
                elif p[0]<point[1]:
                    numA[1]=a
                    pA[0]=p[0]
                elif p[0]<point[2]:
                    numB=a
                    pA[0]=p[0]
                elif p[0]<point[3]:
                    numC[0]=a
                    pA[0]=p[0]
                else:
                    numC[1]=a
                    pA[0]=p[0]
            a+=1 
        if numA[0] == None:
            numA[0] = 0
        if numC[1] == None:
            numC[1] = numC[0]
            numC[0] = 0
        A=10*numA[0]+numA[1]
        C=10*numC[0]+numC[1]
        if A+numB==C:
            py.click(529,727)
        elif A-numB==C:
            py.click(658,727)
        elif A*numB==C:
            py.click(808,727)
        elif A/numB==C:
            py.click(933,727)
        time2=time.clock()
        if time2-time1>=57:
            break
    if w != 2:
        time.sleep(6)
        py.click(488,658)
        time.sleep(1)
    else:
        break
    











