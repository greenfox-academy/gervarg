import java.util.Scanner;

public class Exercises_18_OddEven {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        // Write a program that reads a number from the standard input,
        // Then prints "Odd" if the number is odd, or "Even" if it is even.
        int number = scanner.nextInt();
        if (number % 2 == 0) {
            System.out.println("Even");
        } else {
            System.out.println("Odd");
        }
    }
}
