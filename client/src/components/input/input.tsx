import styles from './input.module.css';

interface InputProps {
  className?: string;
  placeholder?: string;
  type?: string;
  value?: string;
  setValue?: (value: string) => void;
}

/**
 * Styled input box component.
 * 
 * @param placeholder Placeholder value
 * @param className Optional styling for input box
 */
const Input: React.FC<InputProps> = ({
  placeholder,
  className = "",
  type,
  value,
  setValue
}) => {
  return (
    <input
      className={`${styles.input} ${className}`}
      placeholder={placeholder}
      value={value}
      onChange={(e) => setValue && setValue(e.target.value)}
      type={type}
    />
  );
}

export default Input;