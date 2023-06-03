import b

class TskWindow(b.widgets.window):
    context = b.TskWindowContext
    print(context.callback())

#
#
# TskWindow = TskWindow()
# TskWindow.pyInit()
