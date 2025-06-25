import styles from "./label.module.css";

interface LabelProps {
  children: React.ReactNode;
  className?: string;
}

const Label: React.FC<LabelProps> = ({ children, className = "" }) => {
  return <span className={`${styles.label} ${className}`}>{children}</span>;
};

export default Label;
