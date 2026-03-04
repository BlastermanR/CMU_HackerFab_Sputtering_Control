def calculate_crc(input_string):
    """
    Calculates CRC by summing ASCII values of each character
    and taking modulo 256 of the total sum.
    """
    ascii_sum = sum(ord(char) for char in input_string)
    crc = ascii_sum % 256
    return crc

if __name__ == "__main__":
    user_input = input("Enter a string of ASCII digits: ")
    crc_result = calculate_crc(user_input)
    print(f"CRC (mod 256 sum of ASCII values): {crc_result}")
