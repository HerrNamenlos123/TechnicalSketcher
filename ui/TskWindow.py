import b

class TskWindow():
    context = b.TskWindowContext

    def render(self):
        pass

TskWindow = TskWindow()
TskWindow.context.init(TskWindow.render)