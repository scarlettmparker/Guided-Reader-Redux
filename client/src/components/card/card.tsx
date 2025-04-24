import React from "react";
import styles from "./card.module.css";

interface CardProps {
  children?: React.ReactNode;
  className?: string;
}

export const Card: React.FC<CardProps> = ({ children, className = "" }) => {
  return <div className={`${styles.card} ${className}`}>{children}</div>;
};

interface CardHeaderProps {
  children?: React.ReactNode;
  className?: string;
}

export const CardHeader: React.FC<CardHeaderProps> = ({
  children,
  className = "",
}) => {
  return <div className={`${styles.card_header} ${className}`}>{children}</div>;
};

interface CardBodyProps {
  children?: React.ReactNode;
  className?: string;
}

export const CardBody: React.FC<CardBodyProps> = ({
  children,
  className = "",
}) => {
  return <div className={`${styles.card_body} ${className}`}>{children}</div>;
};

interface CardFooterProps {
  children?: React.ReactNode;
  className?: string;
}

export const CardFooter: React.FC<CardFooterProps> = ({
  children,
  className = "",
}) => {
  return <div className={`${styles.card_footer} ${className}`}>{children}</div>;
};
