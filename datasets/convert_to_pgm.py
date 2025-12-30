from PIL import Image
import sys
import os
import argparse


def convert_images_to_pgm(image_path, pgm_path):
    """
    Converts image files to PGM format using Pillow.
    """
    files = os.listdir(image_path)
    files.sort()

    if not os.path.exists(pgm_path):
    	os.mkdir(pgm_path)

    for f in files:
        try:
            # Open the input image
            with Image.open(os.path.join(image_path, f)) as img:
                # PGM is a grayscale format. Convert the image to grayscale ('L' mode) 
                # to ensure compatibility, even if the original image is color.
                if img.mode != 'L' and img.mode != 'I': # 'L' for 8-bit, 'I' for 16-bit grayscale
                    print(f"Original image mode {img.mode}, converting to grayscale.")
                    img = img.convert('L')
                
                # Save the image in PGM format (Pillow handles the extension automatically)
                out_f = "_".join(f.split(".")[:-1]) + ".pgm"
                img.save(os.path.join(pgm_path, out_f))
                print(f"Successfully converted '{os.path.join(image_path, f)}' to '{os.path.join(pgm_path, out_f)}'")

        except FileNotFoundError:
            print(f"Error: The file '{os.path.join(image_path, f)}' was not found.")
            #sys.exit(1)
        except Exception as e:
            print(f"An error occurred: {e}")
            #sys.exit(1)

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("--input_folder", type=str, required=True, help="The path to the folder containing the input images to convert")
	parser.add_argument("--output_folder", type=str, required=True, help="The output folder")

	args = parser.parse_args()

	convert_images_to_pgm(args.input_folder, args.output_folder)


if __name__ == "__main__":
	main()