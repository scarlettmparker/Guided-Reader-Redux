import styles from "./button.module.css";

interface ButtonProps {
  children?: React.ReactNode | string;
  className?: string;
  variant?: "dark" | "light";
  onClick?: () => void;
  onMouseOver?: () => void;
  onMouseLeave?: () => void;
  disabled?: boolean;
}

const Button: React.FC<ButtonProps> = ({
  children,
  className = "",
  variant = "dark",
  onClick,
  onMouseOver,
  onMouseLeave,
  disabled = false,
}) => {
  const handleMouseUp = (event: React.MouseEvent<HTMLButtonElement>) => {
    event.currentTarget.blur();
  };

  const variantClass = variant === "light" ? styles.light : styles.dark;

  return (
    <button
      className={`${styles.button} ${variantClass} ${className}`}
      onClick={onClick}
      onMouseOver={onMouseOver}
      onMouseLeave={onMouseLeave}
      onMouseUp={(e) => {
        handleMouseUp(e);
      }}
      disabled={disabled}
    >
      {children}
    </button>
  );
};

export default Button;
