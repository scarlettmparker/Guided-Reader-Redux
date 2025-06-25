import styles from "./icon.module.css";

interface IconProps {
  src: string;
  size?: "small" | "medium" | "large" | "xl";
  hovered?: boolean;
}

const Icon: React.FC<IconProps> = ({ src, size = "medium", hovered }) => {
  const sizeClass = size !== "medium" ? styles[size] : "";
  const hoverClass = hovered ? styles.hover : "";
  const className = `${styles.icon} ${sizeClass} ${hoverClass}`.trim();

  return <img src={src} className={className} alt="" draggable={false} />;
};

export default Icon;
