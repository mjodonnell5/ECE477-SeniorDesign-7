import customtkinter as ctk
import os
import json
import serial
import serial.tools.list_ports
from tkinter import simpledialog, messagebox, filedialog
import tkinter as tk

FLASHCARD_DIR = "flashcards"
FONT = ("Verdana", 14)
MAX_FILE_NAME_LENGTH = 13
MAX_DEF_LENGTH = 250

# Ensure flashcard directory exists
if not os.path.exists(FLASHCARD_DIR):
    os.makedirs(FLASHCARD_DIR)

# Set theme
ctk.set_appearance_mode("Dark")  # "Light", "Dark", or "System"
ctk.set_default_color_theme("green")  # Options: "blue", "green", "dark-blue"

class FlashcardApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Flashcard Manager")
        self.geometry(f"{self.winfo_screenwidth()}x{self.winfo_screenheight()}+0+0")
        # self.attributes("-fullscreen", True)
        # self.bind("<Escape>", lambda e: self.attributes("-fullscreen", False))


        # self.container = ctk.CTkFrame(self)
        # self.container.pack(fill="both", expand=True)

        self.container = ctk.CTkFrame(self)
        self.container.grid(row=0, column=0, sticky="nsew")

        # self.center_frame = tk.Frame(self)
        # self.center_frame.grid(row=0, column=0, sticky="nsew")
        # self.center_frame.grid_rowconfigure(0, weight=1)
        # self.center_frame.grid_columnconfigure(0, weight=1)


        # Ensure the parent widget resizes properly
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self.container.grid_rowconfigure(2, weight=1)
        self.container.grid_columnconfigure(0, weight=1)

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

    def create_new_set (self):
        # reset EditPage and show blank page
        edit_page = self.pages[EditPage]
        edit_page.set_name_entry.delete(0, "end")
        edit_page.flashcards = [{"term": "", "definition": ""}, {"term": "", "definition": ""},{"term": "", "definition": ""}]
        edit_page.refresh_flashcards()
        self.show_page(EditPage)

class MainPage(ctk.CTkFrame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        ctk.CTkLabel(self, text="E-ink Flashcard Manager", font=(FONT, 18)).pack(pady=10)

        self.flashcard_list_frame = ctk.CTkFrame(self)
        self.flashcard_list_frame.pack(pady=5, fill="both", expand=True)

        self.refresh_list()

        ctk.CTkButton(self, text="Create New Set", font=FONT, fg_color="darkgreen", text_color="white",
              command=self.controller.create_new_set).pack(pady=5)


    def refresh_list(self):
        """Refresh the list of flashcards."""
        for widget in self.flashcard_list_frame.winfo_children():
            widget.destroy()

        files = [f for f in os.listdir(FLASHCARD_DIR) if not f.endswith(".txt")]

        for filename in files:
            row = ctk.CTkFrame(self.flashcard_list_frame)
            row.pack(fill="y", pady=2)

            label = ctk.CTkLabel(row, text=filename.capitalize(), font=FONT, width=100, anchor="w")
            label.grid(row=0, column=0, padx=20, sticky="w")  # Left-align label

            edit = ctk.CTkButton(row, text="Edit", fg_color="darkblue", text_color="white",
                                command=lambda f=filename: self.controller.pages[EditPage].load_flashcard_set(f))
            edit.grid(row=0, column=1, padx=5)

            upload = ctk.CTkButton(row, text="Upload", fg_color="darkblue", text_color="white",
                                    command=lambda f=filename: self.upload_to_device(f))
            upload.grid(row=0, column=2, padx=5)

            delete = ctk.CTkButton(row, text="Delete", fg_color="red", text_color="white",
                                    command=lambda f=filename: self.delete_file(f))
            delete.grid(row=0, column=3, padx=5)

    def delete_file(self, filename):
        """Delete a flashcard set."""
        file_path = os.path.join(FLASHCARD_DIR, filename)

        if not os.path.exists(file_path):
            messagebox.showerror("Error", "File not found!")
            return

        confirm = messagebox.askyesno("Confirm Delete", f"Are you sure you want to delete '{filename}'?")
        if confirm:
            os.remove(file_path)
            self.refresh_list()

    def upload_to_device(self, filename):
        """Upload a flashcard set to the device."""
        filename_path = os.path.join(FLASHCARD_DIR, filename)

        #old version
        # with open(filename_path, "r", encoding="utf-8") as file:
        #     data_payload = file.read()

        # new version: loading the data and also getting number of cards in the set
        with open (filename_path, "r", encoding="utf-8") as file:
            data = json.load(file)
            num_cards = len(data.get("flashcards",[]))
            data_payload = json.dumps(data)

        formatted_filename = f"{filename}${num_cards}"

        print(formatted_filename.encode() + b'\xBC' + data_payload.encode() + b'\x00')

        ports = [port.device for port in serial.tools.list_ports.comports()]

        if not ports:
            messagebox.showerror("Error", "No serial devices found!")
            return

        selected_port = ports[0] if len(ports) == 1 else simpledialog.askstring("Select COM Port", f"Available ports: {', '.join(ports)}")

        


        if selected_port:
            try:
                with serial.Serial(selected_port, baudrate=9600, timeout=2) as ser:
                    ser.write(formatted_filename.encode() + b'\xBC' + data_payload.encode() + b'\x00')
                    messagebox.showinfo("Success", f"Flashcard set uploaded to {selected_port}!")
            except serial.SerialException:
                messagebox.showerror("Error", "Failed to connect to device. Check your COM port.")


class EditPage(ctk.CTkFrame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        ctk.CTkLabel(self, text="Edit Flashcard Set", font=FONT).pack(pady=10)
        # Create a frame to hold the label and entry box
        name_frame = ctk.CTkFrame(self)
        name_frame.pack(pady=5, padx=10, anchor="n")  # Align to the left

        # Place label and entry in the frame
        self.set_name_label = ctk.CTkLabel(name_frame, text="Set Name:", font=FONT)
        self.set_name_label.pack(side="left", padx=5)

        self.set_name_entry = ctk.CTkEntry(name_frame, font=FONT, width=200)
        self.set_name_entry.pack(side="left",  expand=True, fill="x")

        # self.flashcard_frame = ctk.CTkFrame(self)
        # self.flashcard_frame.pack(fill="both", expand=True)

        # Create a scrollable frame for flashcards
        self.flashcard_scrollable_frame = ctk.CTkScrollableFrame(self, width=800, height=450)
        self.flashcard_scrollable_frame.pack(fill="both", expand=True, padx=10, pady=10)


        ctk.CTkButton(self, text="Add Flashcard", fg_color="gray", command=self.add_flashcard).pack(pady=5)
        ctk.CTkButton(self, text="Save & Back", fg_color="green", command=self.save_and_back).pack(pady=5)

        self.flashcards = []



    def load_flashcard_set(self, filename):
        """Load existing flashcard set into the editor."""
        self.set_name_entry.delete(0, "end")
        self.set_name_entry.insert(0, filename.replace(".txt", ""))

        filename_path = os.path.join(FLASHCARD_DIR, filename)

        with open(filename_path, "r", encoding="utf-8") as file:
            data = json.load(file)

        self.flashcards = data.get("flashcards", [])
        
        # adding 2 empty flashcards to get user started
        # for i in range (3):
        #     self.flashcards.append({"term": "", "definition": ""})  # Add an empty flashcard entry

        self.refresh_flashcards()

        self.controller.show_page(EditPage)

    def add_flashcard(self):
        self.save_current_flashcards()
        """Add an empty flashcard entry."""

        #don't refresh the entire screen, just add new row
        new_flashcard = {"term": "", "definition": ""}
        self.flashcards.append(new_flashcard)

        # Create a new row for the flashcard
        row = ctk.CTkFrame(self.flashcard_scrollable_frame)
        row.pack(fill="y", pady=2)

        # Add row number label
        row_number_label = ctk.CTkLabel(row, text=f"{len(self.flashcards)}.", font=FONT, width=30, anchor="w")
        row_number_label.pack(side="left", padx=5)

        # Add term entry
        term_entry = ctk.CTkEntry(row, font=FONT, width=200)
        term_entry.pack(side="left", padx=5)

        # Add definition entry
        definition_entry = ctk.CTkTextbox(row, font=FONT, width=500, height=50, wrap="word")
        definition_entry.pack(side="left", padx=5)

        # Add delete button
        ctk.CTkButton(row, text="X", fg_color="red", command=lambda idx=len(self.flashcards) - 1: self.delete_flashcard(idx)).pack(side="left", padx=5)

        # Store references to the term and definition entries
        new_flashcard["term_entry"] = term_entry
        new_flashcard["definition_entry"] = definition_entry

        # self.refresh_flashcards() #refreshed the entire screen, glitchy

    def limit_definition_length(self, entry):
        """Limit the length of the definition to MAX_DEF_LENGTH."""
        if len(entry.get()) > MAX_DEF_LENGTH:
            entry.delete(MAX_DEF_LENGTH, "end")

    def adjust_textbox_height(self, entry):
        # Get the total number of lines in the Text widget
        num_lines = int(entry.index('end-1c').split('.')[0])

        # Set the height dynamically (minimum 1 line)
        entry.config(height=max(1, num_lines))

    def on_text_change(self, entry):
        # Get current text
        text = entry.get("1.0", "end-1c")  # Get all text (excluding the trailing newline)

        # Enforce 100-character limit
        if len(text) > 100:
            entry.delete("1.0", "end")  # Clear text
            entry.insert("1.0", text[:100])  # Insert only the first 100 chars

        # Adjust the textbox height dynamically
        self.adjust_textbox_height(entry)

    def refresh_flashcards(self):
        """Update flashcard list in the editor."""
        for widget in self.flashcard_scrollable_frame.winfo_children():
            widget.destroy()

        for i, flashcard in enumerate(self.flashcards):
            # row = ctk.CTkFrame(self.flashcard_frame)
            row = ctk.CTkFrame(self.flashcard_scrollable_frame)

            row.pack(fill="y", pady=2)

            # Add row number label
            row_number_label = ctk.CTkLabel(row, text=f"{i + 1}.", font=FONT, width=30, anchor="w")
            row_number_label.pack(side="left", padx=5)

            term_entry = ctk.CTkEntry(row, font=FONT, width= 200)
            term_entry.insert(0, flashcard["term"])
            term_entry.pack(side="left", padx=5)

            #adding limit to definition length
            definition_entry = ctk.CTkTextbox(row, font=FONT, width=500, height=50, wrap ="word")
            definition_entry.insert("1.0", flashcard["definition"])  # Insert the existing definition text
            definition_entry.pack(side="left", padx=5)
            definition_entry.bind("<KeyRelease>", lambda e, entry=definition_entry: self.limit_definition_length(entry))


            ctk.CTkButton(row, text="X", fg_color="red", command=lambda idx=i: self.delete_flashcard(idx)).pack(side="left", padx=5)

            flashcard["term_entry"] = term_entry
            flashcard["definition_entry"] = definition_entry

    def delete_flashcard(self, index):
        self.save_current_flashcards() # fist save the current flashcards
        """Delete a flashcard."""
      
        del self.flashcards[index]
        # If the index is the last one, remove the last widget directly
        if index == len(self.flashcards):  # Check if the index is the last one
            last_widget = self.flashcard_scrollable_frame.winfo_children()[-1]
            last_widget.destroy()
        else:
            # Otherwise, refresh the entire flashcard list
            self.refresh_flashcards()
        # self.refresh_flashcards()

    def save_and_back(self):
        """Save flashcard set and return to main menu."""
        set_name = self.set_name_entry.get().strip()
        if not set_name:
            self.controller.pages[MainPage].refresh_list()
            self.controller.show_page(MainPage)
            return
        
        if len(set_name) > MAX_FILE_NAME_LENGTH:
            messagebox.showerror("Error", f"Set name must be at most {MAX_FILE_NAME_LENGTH} characters long.")
            return

        filename = os.path.join(FLASHCARD_DIR, f"{set_name}")

        # flashcard_data = [{"term": fc["term_entry"].get(), "definition": fc["definition_entry"].get()} for fc in self.flashcards]
        
        # only save flashcards with actual data
        # flashcard_data = [
        #     {"term": fc["term_entry"].get(), "definition": fc["definition_entry"].get()}
        #     for fc in self.flashcards
        #     if fc["term_entry"].get().strip() and fc["definition_entry"].get().strip()
        # ]
        flashcard_data = [
        {
            "term": fc["term_entry"].get().strip(),
            "definition": fc["definition_entry"].get("1.0", "end-1c").strip()  # Use "1.0" and "end-1c" for CTkTextbox
        }
        for fc in self.flashcards
        if fc["term_entry"].get().strip() and fc["definition_entry"].get("1.0", "end-1c").strip()
    ]

        data = {"flashcardSetName": set_name, "flashcards": flashcard_data} # fix to add set name to json so it can be parsed

        with open(filename, "w", encoding="utf-8") as file:
            json.dump(data, file, indent=4)

        self.controller.pages[MainPage].refresh_list()
        self.controller.show_page(MainPage)

    def save_current_flashcards(self):
        """Save the current text in all term/definition boxes before any modification."""
        for flashcard in self.flashcards:
            flashcard["term"] = flashcard["term_entry"].get().strip()
            flashcard["definition"] = flashcard["definition_entry"].get("1.0", "end-1c").strip()

if __name__ == "__main__":
    app = FlashcardApp()
    # app.state("zoomed")
    app.geometry(f"{app.winfo_screenwidth()}x{app.winfo_screenheight()}+0+0")

    app.mainloop()
    
    
