import tkinter as tk
from tkinter import simpledialog, messagebox, filedialog
import json
import serial
import serial.tools.list_ports
import os

FLASHCARD_DIR = "flashcards"
FONT= "Verdana"

# Ensure the directory exists
if not os.path.exists(FLASHCARD_DIR):
    os.makedirs(FLASHCARD_DIR)

class FlashcardApp(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Flashcard Manager")
        self.geometry("600x400")

        self.container = tk.Frame(self)
        self.container.pack(fill="both", expand=True)

        self.pages = {}

        for Page in (MainPage, EditPage):
            page = Page(parent=self.container, controller=self)
            self.pages[Page] = page
            page.grid(row=0, column=0, sticky="nsew")

        self.show_page(MainPage)

    def show_page(self, page_class):
        """Raise the selected page to the front."""
        page = self.pages[page_class]
        page.tkraise()

class MainPage(tk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        self.configure(bg="lightgrey")

        tk.Label(self, text="E-ink Flashcard Manager", font=(FONT, 16), bg="lightgrey").pack(pady=10)

        self.flashcard_list_frame = tk.Frame(self, bg="lightgrey")
        self.flashcard_list_frame.pack(pady=5, fill="both", expand=True)

        self.refresh_list()

        tk.Button(self, text="Create New Set", font= FONT,   bg="darkgreen", fg = "white", borderwidth=1, highlightthickness=0, padx=18, pady=5,
                  command=lambda: controller.show_page(EditPage)).pack(pady=5)
        
        
        

#   Load all flashcard sets and display them.
    def refresh_list(self): 
        for widget in self.flashcard_list_frame.winfo_children():
            widget.destroy()

        files = [f for f in os.listdir(FLASHCARD_DIR) if f.endswith(".txt")]

        for filename in files:
            row = tk.Frame(self.flashcard_list_frame, bg="lightgrey")
            row.pack(fill="x", pady=2)

            word = filename.split(".") # getting ride of .txt extension

            tk.Label(row, text=word[0].capitalize(), font=(FONT, 12), width=20, anchor="w", bg="lightgrey").pack(side="left")

            tk.Button(row, text="Edit",  bg="darkblue", fg = "white", borderwidth=1, highlightthickness=0, padx=18, pady=5,
                      command=lambda f=filename: self.controller.pages[EditPage].load_flashcard_set(f)).pack(side="left", padx=5)
            
            tk.Button(row, text="Upload",  bg="darkblue", fg = "white", borderwidth=1, highlightthickness=0, padx=10, pady=5,
                      command=lambda f=filename: self.upload_to_device(f)).pack(side="left", padx=5)
            
            tk.Button(row, text="Delete", bg="darkblue", fg = "white", borderwidth=1, highlightthickness=0, padx=10, pady=5,
                      command=lambda f=filename: self.delete_file(f)).pack(side="left", padx=5)

# delete a flashcard set
    def delete_file(self, filename):
        file_path = os.path.join(FLASHCARD_DIR, filename)

        if not os.path.exists(file_path):
            messagebox.showerror("Error", "File not found!")
            return

        # Confirm with user before deleting
        confirm = messagebox.askyesno("Confirm Delete", f"Are you sure you want to delete '{filename}'?")
        if not confirm:
            return

        try:
            os.remove(file_path)  # Delete the file
            messagebox.showinfo("Success", f"'{filename}' has been deleted.")
            self.refresh_list()  # Refresh the UI
        except Exception as e:
            messagebox.showerror("Error", f"Failed to delete file: {str(e)}")

# upload a set to the device
    def upload_to_device(self, filename = None):
        if not filename:  # If filename is not provided, ask for one
            filename_path = filedialog.askopenfilename(initialdir=FLASHCARD_DIR, title="Select Flashcard Set", filetypes=[("Text Files", "*.txt")])
            if not filename_path:
                return
            filename = os.path.basename(filename_path)
        else:
            filename_path = os.path.join(FLASHCARD_DIR, filename)  # Use the provided filename
    
        # Load flashcard data from file
        with open(filename_path, "r", encoding="utf-8") as file:
            # flashcards = json.load(file)  # Read JSON data
            data_payload = file.read()  # Read as raw text
    
        print(filename.encode() + b'\xBC' + data_payload.encode() + b'\x00')

        # Create JSON packet with filename and data
        ports = [port.device for port in serial.tools.list_ports.comports()]

        if not ports:
            messagebox.showerror("Error", "No serial devices found!")
            return
        
        # if only 1 port, just choose that
        if len(ports) == 1:
            selected_port = ports[0]
        else:
            selected_port = simpledialog.askstring("Select COM Port", f"Available ports {', '.join(ports)}\n Enter COM port")

        if not selected_port:
            return
        
        #actually send the data
        try:
            with serial.Serial(selected_port, baudrate=9600, timeout=2) as ser:
                ser.write(filename.encode() + b'\xBC' + data_payload.encode()+ b'\x00')
                messagebox.showinfo("Success", f"Flashcard set uploaded to {selected_port}!")
        except serial.SerialException:
            messagebox.showerror("Error", "Failed to connected to device. Check your COM port.")

class EditPage(tk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        self.configure(bg="lightyellow")

        tk.Label(self, text="Edit Flashcard Set", font=(FONT, 16), bg="lightyellow").pack(pady=10)

        self.set_name_entry = tk.Entry(self, font=(FONT, 14))
        self.set_name_entry.pack(pady=5)

        self.flashcard_frame = tk.Frame(self, bg="lightyellow")
        self.flashcard_frame.pack(fill="both", expand=True)

        # add flashcard button
        self.add_flashcard_button = tk.Button(self, text="Add Flashcard", bg="lightgray",  borderwidth=1, highlightthickness=0,command=self.add_flashcard)
        self.add_flashcard_button.pack(pady=5)

        # save and back button
        self.save_button = tk.Button(self, text="Save & Back", bg="lightgreen", borderwidth=1, highlightthickness=0, command=self.save_and_back)
        self.save_button.pack(pady=5)

        self.flashcards = []

    def load_flashcard_set(self, filename):
        """Load existing flashcard set into the editor."""
        self.set_name_entry.delete(0, tk.END)
        self.set_name_entry.insert(0, filename.replace(".txt", ""))

        filename_path = os.path.join(FLASHCARD_DIR, filename)

        with open(filename_path, "r", encoding="utf-8") as file:
            data = json.load(file)
        
        self.flashcards = data.get("flashcards", [])
        self.refresh_flashcards()

        self.controller.show_page(EditPage)

    def add_flashcard(self):
        """Add an empty flashcard entry."""
        # save flashcard first
        self.save_current_flashcards()

        # self.just_save()
        self.flashcards.append({"term": "", "definition": ""})
        self.refresh_flashcards()

    def refresh_flashcards(self):
        """Update flashcard list in the editor."""
        for widget in self.flashcard_frame.winfo_children():
            widget.destroy()

        for i, flashcard in enumerate(self.flashcards):
            row = tk.Frame(self.flashcard_frame, bg="lightyellow")
            row.pack(fill="x", pady=2)

            term_entry = tk.Entry(row, font=(FONT, 12), width=20)
            term_entry.insert(0, flashcard["term"])
            term_entry.pack(side="left", padx=5)

            definition_entry = tk.Entry(row, font=(FONT, 12), width=30)
            definition_entry.insert(0, flashcard["definition"])
            definition_entry.pack(side="left", padx=5)


            delete_button = tk.Button(row, text="X", bg="red", borderwidth=1, highlightthickness=0, command=lambda idx=i: self.delete_flashcard(idx))
            delete_button.pack(side="left", padx=5)

            flashcard["term_entry"] = term_entry
            flashcard["definition_entry"] = definition_entry

    def delete_flashcard(self, index):
        """Delete a flashcard."""
        #save all the current flashcards
        self.save_current_flashcards()
        del self.flashcards[index]
        self.refresh_flashcards()

    def save_and_back(self):
        """Save the flashcard set and return to main menu."""
        set_name = self.set_name_entry.get().strip()

        if not set_name:
            messagebox.showerror("Error", "Flashcard set name cannot be empty.")
            return
        
        filename = os.path.join(FLASHCARD_DIR, f"{set_name}.txt")

        flashcard_data = []
        for flashcard in self.flashcards:
            term = flashcard["term_entry"].get().strip()
            definition = flashcard["definition_entry"].get().strip()

            if term and definition:
                flashcard_data.append({"term": term, "definition": definition})

        # data = {"flashcards": flashcard_data}
        data = {"flashcardSetName": set_name, "flashcards": flashcard_data} # fix to add set name to json so it can be parsed


        with open(filename, "w", encoding="utf-8") as file:
            json.dump(data, file, indent=4)

        # messagebox.showinfo("Success", f"Flashcard set '{set_name}' saved!")

        self.controller.pages[MainPage].refresh_list()
        self.controller.show_page(MainPage)

    def just_save(self):
        """Save the flashcard set and return to main menu."""
        set_name = self.set_name_entry.get().strip()

        if not set_name:
            messagebox.showerror("Error", "Flashcard set name cannot be empty.")
            return
        
        filename = os.path.join(FLASHCARD_DIR, f"{set_name}.txt")

        flashcard_data = []
        for flashcard in self.flashcards:
            term = flashcard["term_entry"].get().strip()
            definition = flashcard["definition_entry"].get().strip()

            if term and definition:
                flashcard_data.append({"term": term, "definition": definition})

        data = {"flashcards": flashcard_data}

        with open(filename, "w", encoding="utf-8") as file:
            json.dump(data, file, indent=4)

        self.refresh_flashcards()

    def save_current_flashcards(self):
        """Save the current text in all term/definition boxes before any modification."""
        for flashcard in self.flashcards:
            flashcard["term"] = flashcard["term_entry"].get().strip()
            flashcard["definition"] = flashcard["definition_entry"].get().strip()

# class NewSetPage(tk.Frame):



if __name__ == "__main__":
    app = FlashcardApp()
    app.mainloop()
