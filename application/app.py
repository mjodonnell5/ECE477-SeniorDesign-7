import tkinter as tk
from tkinter import simpledialog, messagebox, filedialog
import json
import serial
import serial.tools.list_ports
import os


FLASHCARD_DIR = "flashcards"

#ensure directory exists
if not os.path.exists(FLASHCARD_DIR):
    os.makedirs(FLASHCARD_DIR)

class FlashcardApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Flashcard Manager")

        root.configure(bg="lightblue") 
        

        #Main Menu
        self.main_label = tk.Label(root, text = "Flashcard Manager", font=("Rockwell", 14), bg="lightblue")
        self.main_label.pack(pady=10)

        self.create_button = tk.Button(root, text= "Create New Set", font="Rockwell", bg="lightpink", command=self.create_new_set)
        self.create_button.pack(pady=5)

        self.edit_button = tk.Button(root, text= "Edit Existing Set", font="Rockwell",bg="lightpink")
        self.edit_button.pack(pady=5)

        self.upload_button = tk.Button(root, text= "Upload a Set", font="Rockwell",bg="lightpink", command=self.upload_to_device)
        self.upload_button.pack(pady=5)

    # CREATE NEW SET
    def create_new_set(self):
        set_name = simpledialog.askstring("New Flashcard Set", "Enter flashcard set name:")
        if not set_name:
            return
        filename = os.path.join(FLASHCARD_DIR, f"{set_name}.txt")
        flashcards = []

        while True: 
            term = simpledialog.askstring("New Flashcard", "Enter Term (or Cancel to finish):")
            if not term:
                break
            definition = simpledialog.askstring("New Flashcard", "Enter Definition for '{term}':")
            flashcards.append({"term": term, "defintion": definition})
        
        data = {"flashcardSetName": set_name, "flashcards": flashcards}

        with open(filename, "w", encoding="utf-8") as file:
            json.dump(data,file,indent=4)

        messagebox.showinfo("Success", f"Flashcard set '{set_name}' saved!")

    # EDIT A SET

    # UPLOAD A SET OVER UART
    def upload_to_device(self):
        filename_path = filedialog.askopenfilename(initialdir = FLASHCARD_DIR, title="Select Flashcard Set", filetypes=[("Text Files", "*.txt")])
        if not filename_path:
            return
        
        # Extract only the filename (without the full path)
        filename = os.path.basename(filename_path)

        # Load flashcard data from file
        with open(filename_path, "r", encoding="utf-8") as file:
            # flashcards = json.load(file)  # Read JSON data
            data_payload = file.read()  # Read as raw text
    
        print(filename.encode() + b'\xBC' + data_payload.encode() + b'\x00')

        
        # Create JSON packet with filename and data
        # data_payload = json.dumps({"data": flashcards["flashcards"]})

        # print(filename.encode() + b'\xBC' + flashcards + b'\x00')

        ports = [port.device for port in serial.tools.list_ports.comports()]

        if not ports:
            messagebox.showerror("Error", "No serial devices found!")
            return
        
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


if __name__ == "__main__":
    root = tk.Tk()
    app = FlashcardApp(root)
    root.mainloop()


