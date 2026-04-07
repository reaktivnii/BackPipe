# this part is the front end of the app
import tkinter as tk

class Front(tk.Frame):
    def __init__(self, parent):
        tk.Frame.__init__(self, parent)

        self.prompt = tk.Label(self, text="Choose files", anchor="w")
        self.entry = tk.Entry(self)
        self.submit = tk.Button(self, text="Submit", command = self.sendAnsw)
        self.output = tk.Label(self, text="")

        self.prompt.pack(side="top", fill="x")
        self.entry.pack(side="top", fill="x", padx=20)
        self.output.pack(side="top", fill="x", expand=True)
        self.submit.pack(side="right")

    def sendAnsw(self):
        p = self.entry.get()
        result = "Success"
        print(f"Info sent: {p}")
        self.output.configure(text=result)

if __name__ == "__main__":
    root = tk.Tk()
    Front(root).pack(fill="both", expand=True)
    root.mainloop()
