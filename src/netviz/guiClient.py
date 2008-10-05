#!/usr/bin/env python

#********************************************************************
#                  guiClient                              
#    
#  Author:                       
#      Jose .C Renteria            
#      jrenteri@arl.army.mil        
#      US Army Research Laboratory      
#      Adelphi, MD     
#                                                                 
#      Copyright @ 2008 US Army Research Laboratory 
#      All Rights Reserved                                         
#      See Copyright.txt or http://www.arl.hpc.mil/ice for details 
#                                                                  
#      This software is distributed WITHOUT ANY WARRANTY; without  
#      even the implied warranty of MERCHANTABILITY or FITNESS     
#      FOR A PARTICULAR PURPOSE.  See the above copyright notice   
#      for more information.                                       
#                                                                  
# *******************************************************************

import Pyro.core
import time
import sys
from Tkinter import *
from FileDialog import *
from string import *
import tkSimpleDialog


class GuiClient:
    def __init__(self, root):
        self.root = root
        self.root.title("MANET Transfer Rates")
        self.root.resizable(width=YES, height=YES)
        self.set_win = Toplevel(self.root)
        self.set_win.title("Setting Editor")
        self.set_win.resizable(width=NO, height=NO)
        self.set_win.withdraw()
        self.set_win.protocol("WM_DELETE_WINDOW",self.root.quit)
        self.width = 800
        self.height = 500
        self.minBarWidth = 3
        self.maxBarWidth = 50
        self.barSep = 5
        self.barWidth = self.minBarWidth
        self.defaultBorder = 40
        self.bottomBuffer = 60
        self.sleep_time = .1
        self.netData = {}
        self.run=True
        self.debug=False
        
        self.threshold = 0
        self.barFactor = 1
        self.thresholdTrans = 0
        self.barFactorTrans = 1
        self.thresholdRec = 0
        self.barFactorRec = 1
        self.bar_color = "red"
        
        self.connect()
        self.__initMenus()
        self.__initCanvas()
        self.getData()

    def __initMenus(self):
        menubar =  Menu(self.root)
        filemenu = Menu(menubar, tearoff=0)
        filemenu.add_command(label="Exit", command=self.goodBye)
        menubar.add_cascade(label="File",underline=0,menu=filemenu)

        typemenu = Menu(menubar, tearoff=0)
        typemenu.add_checkbutton(label='Debug', command=self.updateDebug)
        typemenu.add_command(label="Settings", command=self.set_win.deiconify)
        menubar.add_cascade(label="Options",underline=0,menu=typemenu)
    
        typemenu2 = Menu(menubar, tearoff=0)
        self.dataView = IntVar()
        typemenu2.add_checkbutton(label='Transfer Data ', variable=self.dataView, onvalue=0, command=self.updateDataView)
        self.dataView.set(0)
        typemenu2.add_checkbutton(label='Receive Data', variable=self.dataView, onvalue=1, command=self.updateDataView)
        menubar.add_cascade(label="View",underline=0,menu=typemenu2)
        
        self.root.config(menu=menubar)

        menubar =  Menu(self.set_win)
        filemenu = Menu(menubar, tearoff=0)
        filemenu.add_command(label="Close", command=self.set_win.withdraw)
        menubar.add_cascade(label="File",underline=0,menu=filemenu)
        self.set_win.config(menu=menubar)
        
        self.maxKPS = 200
        self.minKPS = 0
        self.KPSStep= 1
        self.sampleInterval = int((self.maxKPS - self.minKPS)/5)

        self.aframetop = Frame(self.set_win, width=400,height=300, relief=RAISED, borderwidth=2, pady=5)
        #self.aframetop.resizable(width=YES, height=NO)
        self.aframetop.grid(row=0,column=0)
        self.aframemid = Frame(self.set_win, width=400,height=300, relief=RAISED, borderwidth=2, pady=5)
        self.aframemid.grid(row=1,column=0)
        aframebot = Frame(self.set_win, width=400,height=300, relief=RAISED, borderwidth=2, pady=10, padx=66)
        aframebot.grid(row=2,column=0)
        self.thresScale=Scale(self.aframetop,orient=HORIZONTAL,length=300,
                           from_=self.minKPS,to=self.maxKPS,
                           tickinterval=self.sampleInterval,digits=0,
                           resolution=self.KPSStep,label="Threshold")
        self.thresScale.pack()
        self.thresScale.set(self.threshold)

        self.maxScale = 25
        self.minScale = .25
        self.scaleStep= .25
        self.sampleInterval = int((self.maxKPS - self.minKPS)/5)
        self.scaleScale=Scale(self.aframemid,orient=HORIZONTAL,length=300,
                           from_=self.minScale,to=self.maxScale,
                           tickinterval=6,digits=0,
                           resolution=self.scaleStep,label="Scale Factor")
        self.scaleScale.pack()
        self.scaleScale.set(self.barFactor)
        
        self.apply = Button(aframebot, text="Apply Transfer Settings",fg=self.bar_color,
                       command=self.updateSettings)
        self.apply.pack()

        
    def hello(self):
        print "-----------------------------------------------hi"

    def goodBye(self):
        self.run = False
        sys.exit()

    #START#############  canvas  ################################
    def __initCanvas(self):
        border = 2
        frame = Frame(self.root,bd=border, width=self.width, height=self.height)
        frame.pack(fill=BOTH, expand=YES)
        self.canvas = Canvas(frame,bg="black", width=self.width, height=self.height)
        self.canvas.pack(fill=BOTH, expand=YES)
        self.width = self.canvas.winfo_width()
        self.hieght =  self.canvas.winfo_height()
    
    def getData(self):
        # perhaps insteqd of sleep place a timer to tigger call to server
        while self.run:
            time.sleep(self.sleep_time)
            self.netData = self.server.getData()
            self.draw()
            #self.root.update_idletasks() 
            self.root.update()
        self.goodBye()

    def updateSettings(self):
        self.updateThreshold() 
        self.updateBarFactor()
        
    def updateThreshold(self): 
        self.threshold = self.thresScale.get()
        if self.dataView.get():
            self.thresholdTrans = self.threshold
        else:
            self.thresholdRec = self.threshold

    def updateBarFactor(self): 
        self.barFactor = self.scaleScale.get()
        if self.dataView.get():
            self.barFactorTrans = self.barFactor
        else:
            self.barFactorRec = self.barFactor

    def updateDataView(self):
        if self.dataView.get():
            self.bar_color = "blue"
            self.apply.config( text="Apply Receive Settings",fg=self.bar_color)
            self.scaleScale.set(self.barFactorTrans)
            self.thresScale.set(self.thresholdTrans)
        else:
            self.bar_color = "red"
            self.apply.config( text="Apply Transfer Settings",fg=self.bar_color)
            self.scaleScale.set(self.barFactorRec)
            self.thresScale.set(self.thresholdRec)
            
        self.updateSettings()

    def updateDebug(self):
        if self.debug == True:
            self.debug = False
        else:
            self.debug = True
  
    def connect(self):
        try:
            self.server = Pyro.core.getProxyForURI("PYROLOC://localhost:7766/welstats")
        except ValueError:
            print "Was not able to connect to server!"
            exit

    def close(self):
        self.disconnect()
        self.root.quit()

    def draw(self):
        self.canvas.delete(ALL)
        self.draw_allBars()


    def draw_allBars(self):
        
        if self.height !=  self.canvas.winfo_height():
            self.height = self.canvas.winfo_height()
        if self.width !=  self.canvas.winfo_width():
            self.width = self.canvas.winfo_width()

        self.BotLeftX = self.defaultBorder
        self.BotLeftY = self.height - self.defaultBorder - self.bottomBuffer
        self.TopRightX = self.width - self.defaultBorder
        self.TopRightY = self.defaultBorder
        self.realHeight = float(self.BotLeftY - self.defaultBorder)
        self.realWidth =  float(self.width - (2*self.defaultBorder))

        self.buf =1
        self.barMax = self.BotLeftY + self.buf - self.defaultBorder  # kb
        self.barMin = self.threshold
         
        num_bars = len(self.netData)

        # build index of node ids [101, 123, 124, 145], etc
        # Useful if nodeIDs are not in sequential order
        nodeIdIndex=[self.netData[key][0] for key in self.netData]

        #start_x = self.BotLeftX  
        start_y = self.BotLeftY
        for key in self.netData:
            data = self.netData[key]

            # data[0] is node_id
            # data[1] is avg R_rate
            # data[2] is avg T_rate
            # data[3] is  number of samples
            # data[4] is the current R_rate
            # data[5] is the current T_rate

            nodeID = data[0];   # data[0] is node_id
            nodeIDIndex=nodeIdIndex.index(nodeID)+1

            if self.dataView.get():
                cur_rate = data[5]; # data[5] is the current T_rate
                avg_rate = data[2]; # data[2] is avg T_rate
                legend = "Receive Rates:  Average and Current"
            else:
                cur_rate = data[4]; # data[4] is the current R_rate
                avg_rate = data[1]; # data[1] is avg R_rate
                legend = "Transfer Rates:  Average and Current"
            #
            if self.debug:
                print data
            # computer in order, assumes we areviewing all nodes
            start_x = self.BotLeftX + ((self.barWidth + self.barSep)*(nodeIDIndex - 1))
            end_x = start_x + self.barWidth

            # calculate and draw current rate
            if (cur_rate > self.threshold):
                end_y = start_y - ((cur_rate - self.threshold) * self.barFactor) # Transfer rate
            else:
                end_y = start_y
            self.__drawBar(end_x, end_y, start_x, start_y, self.bar_color)

            # calculate and draw avg rate
            if (avg_rate > self.threshold):
                end_y = start_y - ((avg_rate - self.threshold) * self.barFactor) # avg Transfer rate
            else:
                end_y = start_y
            self.__drawBar(end_x, end_y, start_x, start_y, "white", True)
            # draw node ID

            # draw tick and node id for ever 10th node
            id_space=20
            # if nodeIDIndex == 1 or  nodeIDIndex % 10 ==0:
            if nodeIDIndex:
                str_id = '%s' % nodeID
                self.canvas.create_line(start_x+(self.barWidth/2), start_y+self.buf, start_x+(self.barWidth/2), start_y+10,  fill="white")
                # self.canvas.create_text(start_x+2, start_y+20, text=str_id, fill="white")
                for c in str_id: 
                    self.canvas.create_text(start_x+2, start_y+id_space, text=c, fill="white")
                    id_space=id_space+15
                
            #start_x = end_x + self.barSep
            
        # draw bottom barchart line
        lineEnd = self.BotLeftX + ((self.barWidth + self.barSep)*num_bars) 
        self.canvas.create_line(self.BotLeftX-5,self.BotLeftY+self.buf,lineEnd  ,self.BotLeftY+self.buf,  fill="white")
        self.canvas.create_text( lineEnd + 50, self.BotLeftY,  text="Node Ids", fill="white")

        # draw side barlchart line 
        self.canvas.create_line(self.BotLeftX-5,self.BotLeftY+self.buf, self.BotLeftX-5, self.defaultBorder,  fill="white")
        self.canvas.create_text(self.BotLeftX-15, self.defaultBorder-10 , text="kb/s", fill="white")
       
        ticksep = (self.BotLeftY + self.buf - self.defaultBorder)/9
        tick_y = self.BotLeftY + self.buf
        for i in range(0, 10):
            tick_x1 = self.BotLeftX-5
            tick_x2 = self.BotLeftX-10
            self.canvas.create_line(tick_x1, tick_y, tick_x2, tick_y, fill="white")
            kps = int( (self.barMax + self.defaultBorder - tick_y) / self.barFactor + self.threshold)
            str_kps = '%s' % kps
            #str_kps = '0'
            self.canvas.create_text(tick_x2-10, tick_y, text=str_kps, fill="white")
            tick_y = tick_y - ticksep
            
        # draw AVG rate key
        textStart_x =self.width/4
        textStart_y = self.BotLeftY+self.defaultBorder+self.bottomBuffer/3
        self.canvas.create_text(textStart_x, textStart_y , text="Average Rate", fill="white")
        textStart_x = textStart_x + 50
        self.canvas.create_line(textStart_x, textStart_y ,textStart_x+60, textStart_y, fill="white")
        # draw Current rate key
        textStart_x =self.width/2
        textStart_y = self.BotLeftY+self.defaultBorder+self.bottomBuffer/3
        self.canvas.create_text(textStart_x, textStart_y , text="Current Rate", fill="white")
        textStart_x = textStart_x + 50
        self.canvas.create_line(textStart_x, textStart_y ,textStart_x+60, textStart_y, fill=self.bar_color)

        # draw text
        self.canvas.create_text(self.width/2, self.defaultBorder/3 , text=legend, fill="white")
        
    def __drawBar(self, x0,y0, x1, y1, color, empty=False):
        if empty:
            self.canvas.create_rectangle(x0, y0, x1, y1, outline=color,)
        else:
            self.canvas.create_rectangle(x0, y0, x1, y1, outline=color, fill='gray80')


root = Tk()
gui = GuiClient(root)
root.mainloop()
