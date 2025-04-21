import React from "react";
import styles from "./text-list-item.module.css";

interface TextListItemProps {
  className?: string;
  children: React.ReactNode | string;
  onClick: () => void;
  onMouseOver: () => void;
}

const TextListItem: React.FC<TextListItemProps> = ({
  className = "",
  onClick,
  onMouseOver,
  children,
}) => {
  return (
    <div
      className={`${styles.text_list_item} ${className}`}
      onClick={onClick}
      onMouseOver={onMouseOver}
    >
      {children}
    </div>
  );
};

export default TextListItem;
